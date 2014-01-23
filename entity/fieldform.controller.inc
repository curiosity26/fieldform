<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 1/16/14
 * Time: 2:34 PM
 */

class FieldFormController extends EntityAPIControllerExportable {
  public function __construct($entityType = NULL) {
    parent::__construct('fieldform');
  }

  public function hookForms() {
    $forms = array();

    $entities = fieldform_load_multiple();

    foreach ($entities as $entity) {
      $forms['fieldform_'.$entity->entityType().'_'.$entity->identifier().'_form'] = array(
        'callback' => 'entity_view',
        'callback arguments' => array($entity->entityType(), array($entity->identifier()))
      );
    }

    return $forms;
  }

  public function addElement($entity, FieldFormElement $element) {
    $entity->elements[$element->formKey()] = get_object_vars($element);
    dpm($entity->elements);
    $entity->save();
  }

  static protected function processElements($entity, $elements, $view_mode = 'full') {
    $ret = array();

    foreach ($elements as $name => $settings) {
      if (!empty($settings['type'])) {
        $element = fieldform_element_create($settings['type'], $settings);
        $ret[$name] = $element->buildContent($view_mode);
      }
    }
    return $ret;
  }

  static public function buildForm($entity, $form, &$form_state, $view_mode = 'full', $langcode = NULL) {
    $form += self::processElements($entity, $entity->elements, $view_mode);

    $form_state += array(
      'build_info' => array(
        'args' => array($entity)
      ),
      'entity_type' => $entity->entityType()
    );

    $form_state[$entity->entityType()] = $entity;

    return $form;
  }

  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = array()) {

    $form_state = array();
    $form = array();
    $form = self::buildForm($entity, $form, $form_state, $view_mode, $langcode);

    drupal_process_form('fieldform_'.$entity->entityType().'_'.$entity->identifier().'_form', $form, $form_state);

    $content[] = $form;
    return parent::buildContent($entity, $view_mode, $langcode, $content);
  }

  public function validate($entity, &$form, &$form_state) {
    foreach ($entity->elements as $name => $settings) {
      $settings['name'] = $name;
      $element = fieldform_element_create($settings);
      $element->value = $form_state['values'][$element->formKey()];
      $element->validate($form, $form_state);
    }
  }

  public function submit($entity, &$form, &$form_state) {
    foreach ($entity->elements as $name => $settings) {
      $settings['name'] = $name;
      $element = fieldform_element_create($settings);
      $element->value = $form_state['values'][$element->formKey()];
      $element->submit($form, $form_state);
    }

    // TODO: Create a new Result or catch the result to update from session and save new results
  }

  public function save($entity) {
    $entity->changed = REQUEST_TIME;
    if (isset($entity->is_new) || !isset($entity->created)) {
      $entity->created = REQUEST_TIME;
    }
    $entity->module = isset($entity->entityInfo()['module']) ? $entity->entityInfo()['module'] : 'fieldform';
    return parent::save($entity);
  }
}

class FieldFormAdminUIController extends EntityDefaultUIController {
  public function hook_menu() {
    $menu = parent::hook_menu();
    $id_count = count(explode('/', $this->path));
    $wildcard = isset($this->entityInfo['admin ui']['menu wildcard']) ?
      $this->entityInfo['admin ui']['menu wildcard'] : '%entity_object';
    $element_wildcard = function_exists($this->entityType."_element_object_load") ?
      "%{$this->entityType}_element_object" : "%fieldform_element";
    // Add Elements Configuration

    $menu[$this->path."/manage/$wildcard/elements"] = array(
      'title' => 'Elements',
      'page callback' => 'drupal_get_form',
      'page arguments' => array('fieldform_elements_list', $id_count + 1),
      'access callback' => 'fieldform_access',
      'access arguments' => array('view', $id_count + 1),
      'type' => MENU_LOCAL_TASK
    );

    $menu[$this->path."/manage/$wildcard/elements/list"] = array(
      'title' => 'List',
      'type' => MENU_DEFAULT_LOCAL_TASK,
      'weight' => -10
    );

    $menu[$this->path."/manage/$wildcard/elements/$element_wildcard/edit"] = array(
      'title' => 'Edit Elements',
      'load arguments' => array($id_count + 1),
      'page callback' => 'fieldform_element_operations_form',
      'page arguments' => array('edit', $id_count + 1, $id_count + 3),
      'access callback' => 'fieldform_access',
      'access arguments' => array('update', $id_count + 1),
      'type' => MENU_CALLBACK
    );

    $menu[$this->path."/manage/$wildcard/elements/$element_wildcard/delete"] = array(
      'title' => 'Delete Elements',
      'load arguments' => array($id_count + 1),
      'page callback' => 'fieldform_element_operations_form',
      'page arguments' => array('delete', $id_count + 1, $id_count + 3),
      'access callback' => 'fieldform_access',
      'access arguments' => array('delete', $id_count + 1),
      'type' => MENU_CALLBACK
    );

    if (isset($this->entityInfo['admin ui']['file'])) {
      // Add in the include file for the entity form.
      foreach (array("/manage/$wildcard/elements",
                 "/manage/$wildcard/elements/$element_wildcard/edit",
                 "/manage/$wildcard/elements/$element_wildcard/delete") as $path_end) {
        $menu[$this->path . $path_end]['file'] = $this->entityInfo['admin ui']['file'];
        $menu[$this->path . $path_end]['file path'] = isset($this->entityInfo['admin ui']['file path']) ?
          $this->entityInfo['admin ui']['file path'] : drupal_get_path('module', $this->entityInfo['module']);
      }
    }

    return $menu;
  }
}

interface FieldFormElementControllerInterface {
  public function settingsForm($form, &$form_state);
}

class FieldFormElementController implements FieldFormElementControllerInterface {
  protected $elementType;
  protected $elementInfo;

  public function __construct($elementType) {
    $this->elementType = $elementType;
    $this->elementInfo = fieldform_get_info($elementType);
  }

  public function create($values = array()) {
    $class = isset($this->elementInfo['element class']) && class_implements('FieldFormElement') ?
      $this->elementInfo['element class'] : 'FieldFormElement';
    return new $class($this->elementType, $values);
  }

  public function settingsForm($form, &$form_state, FieldFormElement $element = NULL, $op = NULL) {
    $i = 0;
    $values = isset($form_state['values']) ? $form_state['values'] : get_object_vars($element);
    $properties = $element->elementInfo()['properties'];

    $form['title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title'),
      '#required' => TRUE,
      '#default_value' => $values['title'],
      '#weight' => $i
    );

    $form['name'] = array(
      '#type' => 'hidden',
      '#value' => $values['name']
    );

    $form['required'] = array(
      '#type' => 'checkbox',
      '#title' => t('Required'),
      '#default_value' => isset($values['required']) ? (int)($values['required'] == 1) : FALSE,
      '#weight' => $i++
    );

    $form['description'] = array(
      '#type' => 'textarea',
      '#title' => t('Description'),
      '#default_value' => $values['description'],
      '#weight' => $i++
    );

    foreach ($properties as $name => $property) {
      if ($property['type'] !== FIELDFORM_PROPERTY_NONE) {
        $value = isset($values[$name]) ? $values[$name] : $property['value'];

        $form[$name] = array();
        switch ($property['type']) {
          case FIELDFORM_PROPERTY_TEXT:
            $form[$name]['#type'] = 'textarea';
            break;
          case FIELDFORM_PROPERTY_MARKUP:
            $form[$name]['#type'] = 'text_format';
            break;
          case FIELDFORM_PROPERTY_SELECT:
            $form[$name]['#type'] = 'select';
            $form[$name]['#options'] = isset($value) ? $values : $form['options']['#options'];
            break;
          case FIELDFORM_PROPERTY_RADIOS:
            $form[$name]['#type'] = 'radios';
            $form[$name]['#options'] = isset($value) ? $values : $form['options']['#options'];
            break;
          case FIELDFORM_PROPERTY_CHECKBOXES:
            $form[$name]['#type'] = 'checkboxes';
            $form[$name]['#options'] = isset($value) ? $values : $form['options']['#options'];
            break;
          case FIELDFORM_PROPERTY_TABLE:
            $form[$name]['#type'] = 'namevalue_table';
            break;
          case FIELDFORM_PROPERTY_DATE:
            $form[$name]['#type'] = 'date';
            $form[$name]['#default_value'] = isset($value) ?
              $value : array('month' => date('n'), 'day' => array('j'), 'year' => date('Y'));
            break;
          case FIELDFORM_PROPERTY_BOOL:
            $form[$name]['#type'] = 'checkbox';
            $form[$name]['#checked'] = ($property['value'] === TRUE);
            break;
        }

        $form[$name] += array(
          '#type' => 'textfield',
          '#title' => isset($property['title']) ? $property['title'] : t(ucwords(str_replace('_', ' ', $name))),
          '#default_value' => $value,
          '#weight' => $i++
        );
      }
    }

    $form['details'] = array(
      '#type' => 'fieldset',
      '#title' => t('Element Details'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#weight' => $i
    );

    $form['details']['class'] = array(
      '#type' => 'textfield',
      '#title' => t('Class'),
      '#default_value' => $values['class']
    );

    $form['details']['attributes'] = array(
      '#type' => 'namevalue_table',
      '#title' => t('Attributes'),
      '#default_value' => $values['attributes']
    );

    $form['details']['title_display'] = array(
      '#type' => 'select',
      '#title' => t('Title Display'),
      '#options' => array(
        'before' => t('Before'),
        'after' => t('Inline'),
        'invisible' => t('Hide Label')
      ),
      '#default_value' => $values['title_display']
    );

    return $form;
  }


  public function validate(&$form, &$form_state, $element) {
    // TODO: Process validation per the element
  }

  public function submit(&$form, &$form_state, $element) {
    // TODO: Handle any pre-save processes on the element value
  }
}