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
    $data = $entity->data;
    $tmp = array();
    foreach ($data as $name => $value) {
      $tmp[$name] = $value->save();
    }
    $entity->data = $tmp;
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

  static public function add(FieldFormResultEntity $entity, FieldFormElementInterface $element) {
    $data = array(
      'name' => $element->name,
      'type' => $element->elementType(),
      'title' => $element->label(),
      'value' => $element->value,
      'element' => $element
    );
    $result = fieldform_result_create($data['type'], $data);
    $entity->data[$data['name']] = $result;
  }

  static public function update(FieldFormResultEntity $entity, FieldFormElementInterface $element) {
    if (!empty($entity->data[$element->name])) {
      $data = array(
        'name' => $element->name,
        'type' => $element->elementType(),
        'title' => $element->label(),
        'value' => $element->value,
        'element' => $element
      );

      foreach ($data as $key => $value) {
        $entity->data[$element->name][$key] = $value;
      }
      return $entity;
    }
    return FALSE;
  }

  static public function reset(FieldFormResultEntity $entity) {
    $entity->data = array();
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
}

class FieldFormResultDataController implements FieldFormResultDataControllerInterface {
  protected $resultType;
  protected $resultInfo;

  public function __construct($resultType) {
    $this->resultType = $resultType;
    $this->resultInfo = fieldform_result_get_info($resultType);
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

    if (in_array('FieldFormElementInterface', class_implements($result->element))) {
      $element = $result->element->toArray();
      $ret['element'] = $element;
    }

    return $ret;
  }

  static public function value(FieldFormResultDataInterface $result, $set = NULL) {
    return $result->value($set);
  }

  static public function render(FieldFormResultDataInterface $result, $view_mode = 'full') {
    $title = in_array('FieldFormElementInterface', class_implements($result->element)) ?
      $result->element->label() : $result->title;

    return array(
      '#title' => $title,
      '#result' => $result,
      '#theme' => 'fieldform_result'
    );
  }

  static public function save(FieldFormResultDataInterface $result) {
    $data = self::data($result);
    unset($data['element']);
    $data['value'] = $result->value();
    return $data;
  }
}