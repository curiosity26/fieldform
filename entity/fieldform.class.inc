<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 1/16/14
 * Time: 2:34 PM
 */

class FieldFormEntity extends Entity {
  public $elements = array();

  public function __construct($values = array()) {
    parent::__construct($values, 'fieldform');
    if (!is_array($this->elements)) {
      $this->elements = unserialize($this->elements);
    }
  }
}

class FieldFormElement {
  protected $elementType;
  protected $elementInfo = array();

  public function __construct($values = array(), $elementType) {
    $this->elementType = $elementType;
    $this->elementInfo = fieldform_get_info($elementType);

    if (!empty($values)) {
      foreach($values as $key => $value) {
        $this->$key = $value;
      }
    }
  }

  public function elementInfo() {
    return $this->elementInfo;
  }

  public function elementType() {
    return $this->elementType;
  }

  public function buildContent($view = 'full', $content = array()) {
    return fieldform_element_ui_controller($this->elementType, $view)->buildContent($content);
  }
}