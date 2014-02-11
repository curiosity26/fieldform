<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 1/16/14
 * Time: 5:26 PM
 */

class FieldFormResultEntity extends Entity {
  public $data = array();
  public function __construct($values = array()) {
    parent::__construct($values, 'fieldform_result');
  }

  public function user() {
    $class = $this->entityInfo['controller class'];
    return $class::user($this);
  }

  public function add($data) {
    $class = $this->entityInfo['controller class'];
    $class::add($this, $data);
  }

  public function update($data) {
    $class = $this->entityInfo['controller class'];
    return $class::update($this, $data);
  }

  public function getSection(FieldFormEntity $fieldform, &$form, &$form_state) {
    $class = fieldform_result_controller($this->resultType);
    return $class::getSection($this, $fieldform, $form, $form_state);
  }
}

interface FieldFormResultDataInterface {
  public function resultType();
  public function resultInfo();
  public function data();
  public function value($set = NULL);
  public function render($view_mode = 'full');
  public function save();
  public function buildElement(&$element);
  public function processResult(FieldFormResultEntity $entity, FieldFormElementInterface $element, $value);
}

class FieldFormResultData implements FieldFormResultDataInterface {
  protected $_id;
  protected $value;
  protected $resultType;
  protected $resultInfo;
  public $name;

  public function __construct($resultType, $values = array()) {
    foreach ($values as $key => $value) {
      $this->$key = $value;
    }

    $this->resultType = $resultType;
    if (($this->resultInfo = fieldform_result_get_info($resultType)) === FALSE) {
      $this->resultInfo = fieldform_result_get_info('default');
    }
    $this->_id = date('mdyhis').$this->resultType.$this->name;
  }

  public function resultType() {
    return $this->resultType;
  }

  public function resultInfo() {
    return $this->resultInfo;
  }

  public function identifier() {
    $this->$_id;
  }

  public function data() {
    $class = fieldform_result_controller($this->resultType);
    return $class::data($this);
  }

  public function value($set = NULL) {
    if (isset($set)) {
      $this->value = $set;
    }
    return $this->value;
  }

  public function render($view_mode = 'full') {
    $class = fieldform_result_controller($this->resultType);
    return $class::render($this, $view_mode);
  }

  public function save() {
    $class = fieldform_result_controller($this->resultType);
    return $class::save($this);
  }

  public function buildElement(&$element, $value = NULL) {
    $class = fieldform_result_controller($this->resultType);
    $class::buildElement($this, $element, $value);
  }

  public function processResult(FieldFormResultEntity $entity, FieldFormElementInterface $element, $value) {
    $class = fieldform_result_controller($this->resultType);
    return $class::processResult($this, $entity, $element, $value);
  }
}