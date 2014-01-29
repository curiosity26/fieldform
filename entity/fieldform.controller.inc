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

  public function load($ids = array(), $conditions = array()) {
    $entities = parent::load($ids, $conditions);
    foreach ($entities as $entity) {
      if (!empty($entity->elements) && is_array(current($entity->elements))) {
        $elements = array();
        foreach ($entity->elements as $value) {
          $element = is_array($value) ? fieldform_element_create($value['type'], $value) : $value;
          $elements[$element->_id] = $element;
        }
        $entity->elements = $elements;
      }

      if (!is_array($entity->settings)) {
        $entity->settings = array();
      }

      $entity->settings += array(
        'submit_button_text' => t('Submit'),
        'continue_button_text' => t('Continue'),
        'back_button_text' => t('Back')
      );
    }

    return $entities;
  }

  static public function hookForms() {
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

  static public function sortWeights($a, $b) {
    $a_weight = (is_object($a) && isset($a->weight)) ? $a->weight : 0;
    $b_weight = (is_object($b) && isset($b->weight)) ? $b->weight : 0;
    if ($a_weight == $b_weight) {
      return 0;
    }
    return ($a_weight < $b_weight) ? -1 : 1;
  }

  static public function addElement(FieldFormEntity $entity, $element) {
    if (is_array($element)) {
      $element = fieldform_element_create($element['type'], $element);
    }

    if (empty($entity->elements) || empty($element->parents) || count($element->parents) == 1) {
      if ($element->_id) {
        $entity->elements[$element->_id] = $element;
        //uasort($entity->elements, 'self::sortWeights');
        return TRUE;
      }
      return FALSE;
    }

    reset($entity->elements);
    while (($object = current($entity->elements))) {
      if (($object->addElement($element)) !== FALSE) {
        return TRUE;
      }
      next($entity->elements);
    }
    return FALSE;
  }

  static public function updateElement(FieldFormEntity $entity, FieldFormElement $element) {
    if (($old = $entity->findElementByID($element->_id))) {
      $old->updateElement($element->toArray());
    }
  }

  static public function deleteElement(FieldFormEntity $entity, FieldFormElement $element) {
    if (empty($entity->elements)) {
      return FALSE;
    }

    if (array_key_exists($element->_id, $entity->elements)) {
      unset($entity->elements[$element->_id]);
      return TRUE;
    }

    foreach ($entity->elements as &$child) {
      if ($child->deleteElement($element) === TRUE) {
        return TRUE;
      }
    }

    return FALSE;
  }

  static public function hasElement(FieldFormEntity $entity, $id) {
    if (empty($entity->elements)) {
      return FALSE;
    }
    if (in_array($id, array_keys($entity->elements))) {
      return TRUE;
    }
    reset($entity->elements);
    while (($element = current($entity->elements))) {
      if (($element->hasChild($id)) !== FALSE) {
        return TRUE;
      }
      next($entity->elements);
    }
    return FALSE;
  }

  static public function findElementByID(FieldFormEntity $entity, $id) {
    if (empty($entity->elements)) {
      return FALSE;
    }

    if (in_array($id, array_keys($entity->elements))) {
      return $entity->elements[$id];
    }

    reset($entity->elements);
    while ($element = current($entity->elements)) {
      if (($found = $element->findChildByID($id)) !== FALSE) {
        return $found;
      }
      next($entity->elements);
    }

    return FALSE;
  }

  static public function findElementByName(FieldFormEntity $entity, $name) {
    if (empty($entity->elements)) {
      return FALSE;
    }
    reset($entity->elements);
    while ($element = current($entity->elements)) {
      if ($element->name === $name) {
        return $element;
      }

      if (($found = $element->findChildByName($name)) !== FALSE) {
        return $found;
      }
      next($entity->elements);
    }

    return FALSE;
  }

  static public function findElementsByType(FieldFormEntity $entity, $type) {
    $children = &drupal_static(__FUNCTION__, array());

    if (empty($children) || empty($children[$type])) {
      if (empty($entity->elements)) {
        return $children;
      }

      reset($entity->elements);
      $children[$type] = array();
      while ($element = current($entity->elements)) {
        if ($element->elementType() === $type) {
          $children[$type][$element->_id] = $element;
        }

        $found = $element->findChildrenByType($type);
        $children[$type] += $found;

        next($entity->elements);
      }
    }

    return $children[$type];
  }

  static protected function processElements($entity, $elements, $view_mode = 'full') {
    $form = array();

    $page = isset($entity->separator) ? $entity->separator : 0;
    $separators = $entity->findElementsByType('separator');
    $iterator = new ArrayIterator($elements);

    if (!empty($separators)) {
      $current_separator = array_search($page, $elements);
      $iterator->seek(array_search($current_separator, $elements));
    }

    while($iterator->valid()) {
      $element = $iterator->current();
      if ($element->elementType() === 'separator') {
        $element->page = ++$page;
        $form[$element->name] = $element->buildElement($view_mode);
        break;
      }
      else {
        $form[$element->name] = $element->buildElement($view_mode);
      }
      $iterator->next();
    }

    $form += array(
      'submit' => array(
        '#type' => 'submit',
        '#value' => $entity->settings['submit_button_text']
      )
    );

    return $form;
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

    $form_state = form_state_defaults();
    $form = array();
    $form = self::buildForm($entity, $form, $form_state, $view_mode, $langcode);

    drupal_prepare_form('fieldform_'.$entity->identifier().'_form', $form, $form_state);
    drupal_process_form('fieldform_'.$entity->identifier().'_form', $form, $form_state);

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
    $entity->separator = NULL;
    $values = $form_state['values'];
    unset($values['form_id'], $values['form_build_id'], $values['form_token'], $values['op']);
    $result = isset($form_state['result']) ? $form_state['result'] : entity_create('fieldform_result', array(
      'uid' => $GLOBALS['user']->uid,
      'hostname' => $GLOBALS['user']->hostname
    ));

    foreach ($values as $name => $value) {
      if (($element = $entity->findElementByName($name)) !== FALSE) {
        $element->value = $form_state['values'][$element->formKey()];
        $element->submit($form, $form_state);
        $result->addElement($element);
        if ($element->elementType() === 'separator') {
          $entity->separator = $element->_id;
        }
      }
    }

    entity_save($result->entityType(), $result);
    $form_state['result'] = $result;

    if (isset($entity->separator)) {
      $form_state['rebuild'] = TRUE;
      $form['#rebuild'] = TRUE;
      $form = $entity->buildForm($form, $form_state, $entity, $entity->elements);
    }
    elseif ($entity->settings['completed_behavior'] === 0) {
      $form_state['rebuild'] = TRUE;
      $form['#rebuild'] = TRUE;
      $form = array(
        'complete_message' => array(
          '#markup' => $entity->settings['completed_behavior_0']
        )
      );
    }
    else {
      $form_state['redirect'] = $entity->settings['completed_behavior_1'];
      drupal_redirect_form($form_state);
    }
  }

  public function save($entity) {
    $entity->changed = REQUEST_TIME;
    if (isset($entity->is_new) || !isset($entity->created)) {
      $entity->created = REQUEST_TIME;
    }
    $entity->module = isset($entity->entityInfo()['module']) ? $entity->entityInfo()['module'] : 'fieldform';

    /*
    Convert the elements array from an array of FieldFormElementInterface items to a general array.
    This is useful for at least two reasons:

    1. Compression - Reduces the size of data saved in the database
    2. More accessible to a wider variety of database drivers. MongoDB in specific.
    */

    $clone = $entity->elements;

    if (!empty($clone->elements)) {
      $elements = array();
      foreach ($clone->elements as $element) {
        $elements[$element->name] = $element->save();
      }

      $entity->elements = $elements;
    }

    $return = parent::save($entity);

    $entity->elements = $clone; // Keep the in-memory element the same as it was
    return $return;
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
  static public function addElement(FieldFormElementInterface $element, $child);
  static public function updateElement(FieldFormElementInterface $element, array $values);
  static public function deleteElement(FieldFormElementInterface $element, FieldFormElementInterface $child);
  static public function buildElement(FieldFormElementInterface $element, $view_mode);
  static public function toArray(FieldFormElementInterface $element);
  static public function hasChild(FieldFormElementInterface $element, $id);
  static public function hasChildren(FieldFormElementInterface $element);
  static public function findChildByID(FieldFormElementInterface $element, $id);
  static public function findChildByName(FieldFormElementInterface $element, $name);
  static public function findChildrenByType(FieldFormElementInterface $element, $type);
  static public function settingsForm($form, &$form_state);
  static public function save(FieldFormElementInterface $element);
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

  static public function isElementArray(array $element) {
    $types = array_keys(fieldform_get_info());
    return isset($element['type']) && in_array($element['type'], $types);
  }

  static public function sortWeights($a, $b) {
    $a_weight = (is_object($a) && isset($a->weight)) ? $a->weight : 0;
    $b_weight = (is_object($b) && isset($b->weight)) ? $b->weight : 0;
    if ($a_weight == $b_weight) {
      return 0;
    }
    return ($a_weight < $b_weight) ? -1 : 1;
  }

  static public function addElement(FieldFormElementInterface $element, $child) {
    if (is_array($child)) {
      if (!self::isElementArray($child)) {
        return FALSE;
      }
      $child = fieldform_element_create($child['type'], $child);
    }

    if (empty($child->parents)) {
      // If the parents haven't been set, add it here
      $child->parents = $element->parents;
      $child->parents[] = $child->name;
      $element->children[$child->_id] = $child;
      uasort($element->children, 'self::sortWeights');
      return $element;
    }

    $child_parents = $child->parents;
    array_pop($child_parents);
    if ($element->parents === $child_parents && !in_array($child->_id, $element->children)) {
      $element->children[$child->_id] = $child;
      uasort($element->children, 'self::sortWeights');
      return $element;
    }
    elseif (count($child_parents) > $element->parents) {
      // Child is a grandchild or deeper
      foreach ($element->children as $next) {
        if (($found = $next->addElement($child)) !== FALSE) {
          return $found;
        }
      }
    }

    return FALSE;
  }

  static public function updateElement(FieldFormElementInterface $element, array $values) {
    unset($values['name'], $values['type'], $values['_id'], $values['children']); // Prevent overriding important keys
    foreach ($values as $key => $value) {
      $element->$key = $value;
    }
    return $element;
  }

  static public function deleteElement(FieldFormElementInterface $element, FieldFormElementInterface $child) {
    if ($element->hasChildren() && in_array($child->_id, $element->children)) {
      unset($element->children[$child->_id]);
      return TRUE;
    }
    elseif ($element->hasChildren()) {
      foreach ($element->children as $_id => &$kid) {
        if ($kid->deleteElement($child) === TRUE) {
          return TRUE;
        }
      }
    }

    return FALSE;
  }

  static public function buildElement(FieldFormElementInterface $element, $view_mode = 'full') {
    $array = $element->toArray();
    $form = array();
    foreach ($array as $key => $value) {
      $form['#'.$key] = $value;
    }

    if ($element->hasChildren()) {
      foreach ($element->children as $child) {
        $form[$child->name] = $child->buildElement($child, $view_mode);
      }
    }

    return $form;
  }

  static public function toArray(FieldFormElementInterface $element) {
    $children = $element->children;
    $array = get_object_vars($element);
    unset($array['_id'], $array['parents'], $array['children']);
    return $array;
  }

  static public function hasChild(FieldFormElementInterface $element, $id) {
    return !empty($element->children[$id]);
  }

  static public function hasChildren(FieldFormElementInterface $element) {
    return !empty($element->children);
  }

  static public function findChildByID(FieldFormElementInterface $element, $id) {
    if ($element->hasChild($id)) {
      return $element->children[$id];
    }

    foreach ($element->children as $child) {
      if (($found = $child->findChildById($id)) !== FALSE) {
        return $found;
      }
    }
    return FALSE;
  }

  static public function findChildByName(FieldFormElementInterface $element, $name) {
    foreach ($element->children as $child) {
      if ($child->name === $name) {
        return $child;
      }
      if (($found = $child->findChildByName($name)) !== FALSE) {
        return $found;
      }
    }
    return FALSE;
  }

  static public function findChildrenByType(FieldFormElementInterface $element, $type) {
    $children = array();
    foreach ($element->children as $child) {
      if ($child->elementType() === $type) {
        $children[$child->_id] = $child;
      }
      $found = $child->findChildrenByType($type);
      $children += $found;
    }
    return $children;
  }

  static public function settingsForm($form, &$form_state, FieldFormElementInterface $element = NULL, $op = NULL) {
    $i = 0;
    $values = isset($form_state['input']) && !empty($form_state['input']) ?
      $form_state['input'] : get_object_vars($element);
    $properties = $element->elementInfo()['properties'];

    $form['title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title'),
      '#required' => TRUE,
      '#default_value' => isset($values['title']) ? $values['title'] : '',
      '#weight' => $i
    );

    $form['name'] = array(
      '#type' => 'hidden',
      '#value' => isset($values['name']) ? $values['name'] : ''
    );

    $form['type'] = array(
      '#type' => 'hidden',
      '#value' => isset($values['type']) ? $values['type'] : ''
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
      '#default_value' => isset($values['description']) ? $values['description'] : '',
      '#weight' => $i++
    );

    foreach ($properties as $name => $property) {
      if ($property['type'] !== FIELDFORM_PROPERTY_NONE) {
        $value = NULL;
        if (isset($values[$name])) {
          $value = $values[$name];
        }
        elseif (isset($property['default_value'])) {
          $value = $property['default_value'];
        }
        $form[$name] = array();
        switch ($property['type']) {
          case FIELDFORM_PROPERTY_TEXT:
            $form[$name]['#type'] = 'textarea';
            break;
          case FIELDFORM_PROPERTY_MARKUP:
            $form[$name]['#type'] = 'text_format';
            break;
          case FIELDFORM_PROPERTY_SELECT:
            $value = empty($value) ? array() : $value;
            $form[$name]['#type'] = 'select';
            $form[$name]['#options'] = isset($value) ? $value : $form['options']['#options'];
            break;
          case FIELDFORM_PROPERTY_RADIOS:
            $value = empty($value) ? array() : $value;
            $form[$name]['#type'] = 'radios';
            $form[$name]['#options'] = isset($value) ? $value : $form['options']['#options'];
            break;
          case FIELDFORM_PROPERTY_CHECKBOXES:
            $value = empty($value) ? array() : $value;
            $form[$name]['#type'] = 'checkboxes';
            $form[$name]['#options'] = isset($value) ? $value : $form['options']['#options'];
            break;
          case FIELDFORM_PROPERTY_TABLE:
            $value = empty($value) ? array() : $value;
            $form[$name]['#type'] = 'namevalue_table';
            $form[$name]['#options'] = $value;
            break;
          case FIELDFORM_PROPERTY_DATE:
            $form[$name]['#type'] = 'date';
            $form[$name]['#default_value'] = isset($value) ?
              $value : array('month' => date('n'), 'day' => array('j'), 'year' => date('Y'));
            break;
          case FIELDFORM_PROPERTY_BOOL:
            $form[$name]['#type'] = 'checkbox';
            $form[$name]['#default_value'] = ($value === TRUE);
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

    $class = '';

    if (isset($values['attributes']) && isset($values['attributes']['class'])) {
      $class = $values['attributes']['class'];
      unset($values['attributes']['class']);
    }

    $form['details']['class'] = array(
      '#type' => 'textfield',
      '#title' => t('Class'),
      '#default_value' => $class
    );

    $form['details']['attributes'] = array(
      '#type' => 'namevalue_table',
      '#title' => t('Attributes'),
      '#options' => isset($values['attributes']) ? $values['attributes'] : array()
    );

    $form['details']['title_display'] = array(
      '#type' => 'select',
      '#title' => t('Title Display'),
      '#options' => array(
        'before' => t('Before'),
        'after' => t('Inline'),
        'invisible' => t('Hide Label')
      ),
      '#default_value' => isset($values['title_display']) ? $values['title_display'] : 'before'
    );

    return $form;
  }

  static public function settingsValidateForm(&$form, &$form_state, $entity_type, $entity, $element_type, $element) {
    // Just a placeholder function to extend
  }

  static public function settingsSubmitForm(&$form, &$form_state, $entity_type, $entity, $element_type, $element) {
   if (isset($form_state['input']['attributes']['values'])) {
     $form_state['input']['attributes'] = $form_state['input']['attributes']['values'];
   }
    $form_state['input']['attributes'] = $form_state['input']['attributes'] +
      array('class' => $form_state['input']['class']);
  }

  static public function validate(&$form, &$form_state, $element) {
    // TODO: Process validation per the element
  }

  static public function submit(&$form, &$form_state, $element) {
    // TODO: Handle any pre-save processes on the element value
  }

  static public function save(FieldFormElementInterface $element) {
    $array = $element->toArray();
    if ($element->hasChildren()) {
      foreach ($element->children as $child) {
        $array[$child->name] = $child->save();
      }
    }
    return $array;
  }
}