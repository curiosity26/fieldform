<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 1/16/14
 * Time: 2:34 PM
 */

class FieldFormController extends EntityAPIControllerExportable {
  public function __construct() {
    parent::__construct('fieldform');
  }
}

class FieldFormAdminUIController extends EntityDefaultUIController {
  public function hook_menu() {
    $menu = parent::hook_menu();
    $id_count = count(explode('/', $this->path));
    $wildcard = isset($this->entityInfo['admin ui']['menu wildcard']) ?
      $this->entityInfo['admin ui']['menu wildcard'] : '%entity_object';
    $element_wildcard = function_exists($this->entityType."_element_object") ?
      "%{$this->entityType}_element_object" : "%fieldform_element";

    // Add Elements Configuration

    $menu[$this->path."manage/$wildcard/elements"] = array(
      'title' => 'Elements',
      'page callback' => 'drupal_get_form',
      'page arguments' => array('fieldform_elements_list', $id_count + 2),
      'access callback' => 'fieldform_access',
      'access arguments' => array('view', $id_count + 2),
      'type' => MENU_LOCAL_TASK
    );

    $menu[$this->path."manage/$wildcard/elements/list"] = array(
      'title' => 'List',
      'type' => MENU_DEFAULT_LOCAL_TASK,
      'weight' => -10
    );

    $menu[$this->path."manage/$wildcard/elements/add"] = array(
      'title' => 'Add Elements',
      'page callback' => 'fieldform_element_operations_form',
      'page arguments' => array('add', $id_count + 2),
      'access callback' => 'fieldform_access',
      'access arguments' => array('create', $id_count + 2),
      'type' => MENU_LOCAL_TASK
    );

    $menu[$this->path."manage/$wildcard/elements/$element_wildcard/edit"] = array(
      'title' => 'Edit Elements',
      'load arguments' => array($id_count + 4),
      'page callback' => 'fieldform_element_operations_form',
      'page arguments' => array('edit', $id_count + 2),
      'access callback' => 'fieldform_access',
      'access arguments' => array('update', $id_count + 2),
      'type' => MENU_LOCAL_TASK
    );

    $menu[$this->path."manage/$wildcard/elements/$element_wildcard/delete"] = array(
      'title' => 'Delete Elements',
      'load arguments' => array($id_count + 4),
      'page callback' => 'fieldform_element_operations_form',
      'page arguments' => array('delete', $id_count + 2),
      'access callback' => 'fieldform_access',
      'access arguments' => array('delete', $id_count + 2),
      'type' => MENU_LOCAL_TASK
    );

    if (!empty($this->entityInfo['admin ui']['file'])) {
      // Add in the include file for the entity form.
      foreach (array("manage/$wildcard/elements", "manage/$wildcard/elements/add",
                 "manage/$wildcard/elements/$element_wildcard/edit",
                 "manage/$wildcard/elements/$element_wildcard/delete") as $path_end) {
        $items[$this->path . $path_end]['file'] = $this->entityInfo['admin ui']['file'];
        $items[$this->path . $path_end]['file path'] = isset($this->entityInfo['admin ui']['file path']) ?
          $this->entityInfo['admin ui']['file path'] : drupal_get_path('module', $this->entityInfo['module']);
      }
    }

    return $menu;
  }
}

interface FieldFormElementControllerInterface {
  public function settings();
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
    return new $class($values, $this->elementType);
  }

  public function settings(FieldFormElement $element = NULL) {
    $info = element_info($this->elementType);
    $settings = array();

    if ($info['#input'] === TRUE) {
      $exclude = array(
        '#type' => NULL,
        '#id' => NULL,
        '#theme' => NULL,
        '#process' => NULL,
        '#processed' => NULL,
        '#action'=> NULL,
        '#defaults_loaded' => NULL,
        '#access' => NULL,
        '#after_build' => NULL,
        '#after_build_done' => NULL,
        '#sorted' => NULL,
        '#input' => NULL,
        '#machine_name' => NULL
      );

      $info = array_diff_key($info, $exclude);

      foreach ($info as $key => $setting) {
          $name = str_replace('#', '', $key);
          $settings[$name] = $setting;
        }
    }

    if ($element) {
      $vars = get_object_vars($element);
      $settings = array_merge($settings, $vars);
    }

    return $settings;
  }

  public function settingsForm($form, &$form_state, FieldFormElement $element = NULL, $op = NULL) {
    $settings = $this->settings($element);
    $attributes = $settings['attributes'];

    $i = 0;

    $form['title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title'),
      '#required' => TRUE,
      '#default_value' => empty($form['title']) ?
        $settings['title'] : $form['title']['#default_value'],
      '#weight' => $i
    );

    $form['name'] = array(
      '#type' => 'machine_name',
      '#machine_name' => array(
        'source' => 'title'
      ),
      '#weight' => $i++
    );

    $form['description'] = array(
      '#type' => 'textarea',
      '#title' => t('Description'),
      '#default_value' => empty($form['description']) ?
          $settings['description'] : $form['description']['#default_value']
    );

    unset($settings['title'], $settings['name'], $settings['description'], $settings['attributes']);

    foreach ($settings as $name => $setting) {
      $default_value = empty($form[$name]) ? $setting : $form[$name]['#default_value'];
      $form[$name] = array(
        '#type' => 'textfield',
        '#title' => t(str_replace('_', ' ', ucwords($name))),
        '#default_value' => !empty($form_state['values'][$name]) ?
            $form_state['values'][$name] : $default_value,
        '#weight' => $i++
      );
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
      '#default_value' => fieldform_element_attributes_to_string($attributes)
    );

    $form['details']['attributes'] = array(
      '#type' => 'textarea',
      '#title' => t('Attributes'),
      '#default_value' => fieldform_element_attributes_to_string($attributes)
    );

    $form['details']['required'] = array(
      '#type' => 'checkbox',
      '#title' => t('Required'),
      '#default_value' => (int)(empty($form['required']) ? $settings['required'] :
          $form['required']['#default_value'])
    );

    return $form;
  }
}

interface FieldFormUIInterface {
  public function buildContent(FieldFormElement $element);
  public function view(FieldFormElement $element);
}

class FieldFormElementUIController implements FieldFormUIInterface {
  protected $elementType;
  protected $elementInfo;

  public function __construct($elementType) {
    $this->elementType = $elementType;
    $this->elementInfo = fieldform_get_info($elementType);
  }

  public function buildContent(FieldFormElement $element, $view_mode = 'full', $content = array()) {
    $element->content = $content;

    return $content;
  }

  public function view(FieldFormElement $element, $view_mode = 'full') {
    $content = $this->buildContent($element, $view_mode);

    return drupal_render($content);
  }
}