<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 1/16/14
 * Time: 2:34 PM
 */

class FieldFormEntity extends Entity {
  public $elements = array();
  public $settings = array();

  public function __construct($values = array()) {
    parent::__construct($values, 'fieldform');

    if (!empty($this->elements) && is_array($this->elements)) {
      $elements = array();
      foreach ($this->elements as $value) {
        if (is_string($value)) {
          $value = unserialize($value);
        }
        $element = is_array($value) ? fieldform_element_create($value['type'], $value) : $value;
        $elements[$element->_id] = $element;
      }
      $this->elements = $elements;
    }
  }

  public function addElement($element) {
    $class = entity_get_controller($this->entityType);
    return $class::addElement($this, $element);
  }

  public function updateElement($element) {
    $class = entity_get_controller($this->entityType);
    return $class::updateElement($this, $element);
  }

  public function deleteElement($element) {
    $class = entity_get_controller($this->entityType);
    return $class::deleteElement($this, $element);
  }

  public function hasElement($element) {
    $class = entity_get_controller($this->entityType);
    return $class::hasElement($this, $element);
  }

  public function findElementByID($id) {
    $class = entity_get_controller($this->entityType);
    return $class::findElementByID($this, $id);
  }

  public function findElementByName($name) {
    $class = entity_get_controller($this->entityType);
    return $class::findElementByName($this, $name);
  }

  public function findElementsByType($type) {
    $class = entity_get_controller($this->entityType);
    return $class::findElementsByType($this, $type);
  }

  public function getPreviousSeparatorId($separator) {
    $class = entity_get_controller($this->entityType);
    return $class::getPreviousSeparatorId($this, $separator);
  }

  public function getSeparatorId($form_state) {
    $class = entity_get_controller($this->entityType);
    return $class::getSeparatorId($this, $form_state);
  }

  public function getSection($separator = NULL, &$form_state) {
    $class = entity_get_controller($this->entityType);
    return $class::getSection($this, $separator, $form_state);
  }

  public function processElements(&$form, &$form_state) {
    $class = entity_get_controller($this->entityType);
    $class::processElements($this, $form, $form_state);
  }

  public function buildForm($form, &$form_state) {
    $class = entity_get_controller($this->entityType);
    return $class::buildForm($this, $form, $form_state);
  }

  public function validate(&$form, &$form_state) {
    $class = entity_get_controller($this->entityType);
    $class::validate($this, $form, $form_state);
  }

  public function submit($form, &$form_state) {
    $class = entity_get_controller($this->entityType);
    $class::submit($this, $form, $form_state);
  }

  public function completedFormBehavior($form, &$form_state) {
    $class = entity_get_controller($this->entityType);
    $class::completedFormBehavior($this, $form, $form_state);
  }
}

interface FieldFormElementInterface {
  public function elementInfo();
  public function elementType();
  public function formKey();
  public function addElement($child);
  public function buildElement(FieldFormEntity $entity);
  public function updateElement($values);
  public function deleteElement(FieldFormElementInterface $child);
  public function toArray();
  public function hasChild($id);
  public function hasChildren();
  public function findChildByID($id);
  public function findChildByName($name);
  public function findChildrenByType($type);
  public function save();
}

class FieldFormElement implements FieldFormElementInterface {
  protected $elementType;
  protected $elementInfo = array();
  public $_id;
  public $name;
  public $parents = array();
  public $children = array();

  public function __construct($elementType, $values = array()) {
    $this->elementType = $elementType;
    $this->elementInfo = fieldform_get_info($elementType);
    $this->create($values);
  }

  protected function create($values) {
    $children = array();

    if (!empty($values)) {
      $info = fieldform_get_info();
      $types = array_keys($info);


      foreach($values as $key => $value) {
        if (is_array($value) && isset($value['type']) && in_array($value['type'], $types)) {
          $value['name'] = isset($value['name']) ? $value['name'] : $key;
          $children[] = $value;
          continue;
        }
        elseif (is_object($value) && in_array('FieldFormElementInterface', class_implements($value))) {
          $children[$value->_id] = $value; // Try to retain order by adding it in the order it came in
          continue;
        }
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
    $this->parents = array_unique($this->parents);

    $this->type = $this->elementType;
    if (!isset($this->_id)) {
      $this->_id = date('mdyhis').$this->type.$this->name;
    }

    if (!empty($children)) {
      // Process children after the parent has been processed
      foreach ($children as $child) {
        if (is_array($child)) {
          $child['parents'] = array_merge($this->parents, array($child['name']));
          $element = fieldform_element_controller($child['type'])->create($child);
        }
        else {
          $element = $child;
        }

        $this->children[$element->_id] = $element;
      }
    }
  }

  public function formKey() {
    $parents = array_filter($this->parents);
    return !empty($parents) ? implode('][', $this->parents) : $this->name;
  }

  public function elementInfo() {
    _fieldform_process_info_files($this->elementInfo);
    return $this->elementInfo;
  }

  public function elementType() {
    return $this->elementType;
  }

  public function buildElement(FieldFormEntity $entity, $form = array(), &$form_state = array()) {
    $class = fieldform_element_controller($this->elementType);
    return $class::buildElement($this, $entity, $form, $form_state);
  }

  public function toArray() {
    $class = fieldform_element_controller($this->elementType);
    return $class::toArray($this);
  }

  public function addElement($child) {
    $class = fieldform_element_controller($this->elementType);
    return $class::addElement($this, $child);
  }

  public function updateElement($values) {
    $class = fieldform_element_controller($this->elementType);
    return $class::updateElement($this, $values);
  }

  public function deleteElement(FieldFormElementInterface $child) {
    $class = fieldform_element_controller($this->elementType);
    return $class::deleteElement($this, $child);
  }

  public function hasChild($id) {
    $class = fieldform_element_controller($this->elementType);
    return $class::hasChild($this, $id);
  }

  public function hasChildren() {
    $class = fieldform_element_controller($this->elementType);
    return $class::hasChildren($this);
  }

  public function findChildByID($id) {
    $class = fieldform_element_controller($this->elementType);
    return $class::findChildByID($this, $id);
  }

  public function findChildByName($name) {
    $class = fieldform_element_controller($this->elementType);
    return $class::findChildByName($this, $name);
  }

  public function findChildrenByType($name) {
    $class = fieldform_element_controller($this->elementType);
    return $class::findChildrenByType($this, $name);
  }

  public function validate(&$form, &$form_state) {
    $class = fieldform_element_controller($this->elementType);
    if (method_exists($class, 'validate')) {
      $class->validate($this, $form, $form_state);
    }
  }

  public function submit(&$form, &$form_state) {
    $class = fieldform_element_controller($this->elementType);
    if (method_exists($class, 'submit')) {
      $class->submit($this, $form, $form_state);
    }
  }

  public function save() {
    $class = fieldform_element_controller($this->elementType);
    return $class::save($this);
  }

  public function __sleep() {
    $vars = get_object_vars($this);
    $vars['children'] = array();
    foreach ($this->children as $child) {
      $vars['children'][] = serialize($child);
    }
    $this->children = $vars['children'];
    unset($vars['elementInfo']);
    return drupal_map_assoc(array_keys($vars));
  }

  public function __wakeup() {
    $this->elementInfo = fieldform_get_info($this->elementType);
    $children = $this->children;

    if (!empty($children)) {
      // Process children after the parent has been processed
      foreach ($children as $child) {
        if (is_string($child)) {
          $child = unserialize($child);
        }
        if (!!$child) {
          if (is_array($child)) {
            $child['parents'] = array_merge($this->parents, array($child['name']));
            $element = fieldform_element_controller($child['type'])->create($child);
          }
          else {
            $element = $child;
          }
          dpm(get_class($element).' | '.$element, 'Wakeup Element');
          $this->children[$element->_id] = $element;
        }
      }
    }
  }
}