<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 1/16/14
 * Time: 5:26 PM
 */

class FieldFormResultController extends EntityAPIController {
  public function __construct($entityType) {
    parent::__construct('fieldform_result');
  }

  public function load($ids = array(), $conditions = array()) {
    foreach ($ids as $entity) {
      $entity->form = fieldform_load($entity->form_name);
      if (!isset($entity->module)) {
        $entity->module = 'fieldform';
      }
      if (!empty($entity->data)) {
        $data = $entity->data;
        $tmp = array();
        foreach ($data as $name => $value) {
          $tmp[$name] = fieldform_result_create($value['type'], $value);
          $tmp[$name]['element'] = $entity->form->findElementByName($name);
        }
        $entity->data = $tmp;
      }
    }
  }

  public function save($entity) {
    if ($entity->is_new) {
      $entity->created = REQUEST_TIME;
    }
    $entity->changed = REQUEST_TIME;

    if (!isset($entity->module)) {
      $entity->module = 'fieldform';
    }

    $data = $entity->data;
    if (!empty($data)) {
      $tmp = array();
      foreach ($data as $name => $value) {
        $tmp[$name] = $value->save();
      }
      $entity->data = $tmp;
    }

    parent::save($entity);
    $entity->data = $data;
  }

  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = array()) {
    $data = array();

    foreach ($entity->data as $name => $value) {
      $data[$name] = $value->render($view_mode);
    }

    return $data;
  }

  static public function user(FieldFormResultEntity $entity) {
    return user_load($entity->uid);
  }

  static public function add(FieldFormResultEntity $entity, $data) {
    if (is_array($data) && isset($data['name']) && isset($data['type'])) {
      $data = fieldform_result_create($data['type'], $data);
    }
    dpm($data, 'Add Data');
    if (isset($data->name) && isset($data->value)) {
      $entity->data[$data->name] = $data;
    }
  }

  static public function update(FieldFormResultEntity $entity, $data) {
    dpm($data, 'Update Data');
    if (is_array($data) && isset($data['name']) && isset($data['type'])) {
      $data = fieldform_result_create($data['type'], $data);
    }
    if (!empty($entity->data[$data->name])) {

      foreach ($data as $key => $value) {
        $entity->data[$data->name][$key] = $value;
      }
      return $entity;
    }
    return FALSE;
  }

  static public function reset(FieldFormResultEntity $entity) {
    $entity->data = array();
  }

  static public function buildForm(FieldFormResultEntity $entity, FieldFormEntity $fieldform, &$form, &$form_state) {
    if (!empty($entity->data)) {
      foreach ($entity->data as $result) {
        $element = $fieldform->findElementByName($result->name);
        if (is_array($element->parents)) {
          $ret = drupal_array_get_nested_value($form, $element->parents);
        }
        else {
          $ret = &$form[$element->name];
        }
        $result->buildElement($ret);
      }
    }
  }

  static public function processResults(FieldFormResultEntity $entity, FieldFormEntity $fieldform, $form, $form_state) {
    foreach ($fieldform->elements as $element) {
      $child = $form[$element->name];
      $value = drupal_array_get_nested_value($form_state['values'], $child['#parents']);
      dpm($element, 'Element');
      dpm($value, 'Value');
      $data = array(
        'name' => $element->name,
        'type' => $element->type,
        'value' => $value,
        'title' => $element->label()
      );
      // TODO : Need to process the value per the ResultData controller.
      if (self::update($entity, $data) === FALSE) {
        self::add($entity, $data);
      }
      dpm($entity, 'Result');

      if ($element['#tree'] !== TRUE && count(element_children($element)) > 0) {
        self::processResults($entity, $fieldform, $element, $form_state);
      }
    }
  }

  static public function buildResult(FieldFormResultEntity $entity, FieldFormEntity $fieldform, $form, $form_state) {
    $entity->form_name = $fieldform->identifier();
    if (!isset($entity->uid)) {
      $entity->uid = $GLOBALS['user']->uid;
      $entity->hostname = $GLOBALS['user']->hostname;
    }

    self::processResults($entity, $fieldform, $form, $form_state);

    return $entity;
  }
}

interface FieldFormResultDataControllerInterface {
  public function resultType();
  public function resultInfo();
  public function create($resultType, $values = array());
  static public function data(FieldFormResultDataInterface $result);
  static public function value(FieldFormResultDataInterface $result, $set = NULL);
  static public function render(FieldFormResultDataInterface $result, $view_mode = 'full');
  static public function save(FieldFormResultDataInterface $result);
  static public function buildElement(FieldFormResultDataInterface $result, &$element);
}

class FieldFormResultDataController implements FieldFormResultDataControllerInterface {
  protected $resultType;
  protected $resultInfo;

  public function __construct($resultType) {
    $this->resultType = $resultType;
    if ($this->resultInfo = fieldform_result_get_info($resultType) === FALSE) {
      $this->resultInfo = fieldform_result_get_info('default');
    }
  }

  public function create($resultType, $values = array()) {
    $class = isset($this->resultInfo['result class']) ? $this->resultInfo['result class'] : 'FieldFormResultData';
    return new $class($resultType, $values);
  }

  public function resultType() {
    return $this->resultType;
  }

  public function resultInfo() {
    return $this->resultInfo;
  }

  static public function data(FieldFormResultDataInterface $result) {
    $ret = get_object_vars($result);
    return $ret;
  }

  static public function value(FieldFormResultDataInterface $result, $set = NULL) {
    return $result->value($set);
  }

  static public function render(FieldFormResultDataInterface $result, $view_mode = 'full') {
    return array(
      '#title' => $result->title,
      '#result' => $result,
      '#theme' => 'fieldform_result'
    );
  }

  static public function save(FieldFormResultDataInterface $result) {
    $data = self::data($result);
    $data['value'] = $result->value();
    return $data;
  }

  static public function buildElement(FieldFormResultDataInterface $result, &$element) {
    $value = $result->value();
    if (isset($value)) {
      $element['#default_value'] = $value;
    }
  }
}