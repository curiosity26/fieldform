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
  }

  public function validate(&$form, &$form_state) {
    entity_get_controller($this->entityType)->validate($this, $form, $form_state);
  }

  public function submit(&$form, &$form_state) {
    entity_get_controller($this->entityType)->submit($this, $form, $form_state);
  }
}

class FieldFormElement {
  protected $elementType;
  protected $elementInfo = array();
  public $name;
  public $parents = array();

  public function __construct($elementType, $values = array()) {
    $this->elementType = $elementType;
    $this->elementInfo = fieldform_get_info($elementType);

    if (!empty($values)) {
      foreach($values as $key => $value) {
        $this->$key = $value;
      }
    }

    $this->name = preg_replace('/^[^A-Za-z]+[^A-Za-z0-9_-]+$/', '', $this->name);

    if (!is_array($this->parents) || empty($this->parents)) {
      $this->parents = array($this->name);
    }
    elseif (($flip = array_reverse($this->parents)) !== FALSE && current($flip) !== $this->name) {
      array_unshift($this->parents, $this->name);
    }

    $this->parents = array_filter($this->parents);
    if (!isset($this->type)) {
      $this->type = $elementType;
    }
  }

  public function formKey() {
    $parents = array_filter($this->parents);
    return !empty($parents) ? implode('][', $this->parents) : $this->name;
  }

  public function elementInfo() {
    return $this->elementInfo;
  }

  public function elementType() {
    return $this->elementType;
  }

  public function buildContent($view_mode = 'full') {
    return fieldform_element_controller($this->elementType, $view_mode)->buildContent($this);
  }

  public function hasChild($name) {
    $types = array_keys(fieldform_get_info());
    return isset($this->$name['name']) && (isset($this->$name['type']) && in_array($this->$name['type'], $types));
  }

  public function hasChildren() {
    $keys = get_object_vars($this);
    $types = array_keys(fieldform_get_info());
    foreach ($keys as $value) {
      if (is_array($value) && isset($value['name']) && (isset($value['type']) && in_array($value['type'], $types))) {
        return TRUE;
      }
    }
    return FALSE;
  }

  public function validate(&$form, &$form_state) {
    fieldform_element_controller($this->elementType)->validate($this, $form, $form_state);
  }

  public function submit(&$form, &$form_state) {
    fieldform_element_controller($this->elementType)->submit($this, $form, $form_state);
  }
}