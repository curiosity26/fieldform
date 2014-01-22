<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 1/18/14
 * Time: 3:28 PM
 */

interface FieldFormElementUIInterface {
  public function buildContent(FieldFormElement $element);
}

class FieldFormElementUIController implements FieldFormElementUIInterface {
  protected $elementType;
  protected $elementInfo;

  public function __construct($elementType) {
    $this->elementType = $elementType;
    $this->elementInfo = fieldform_get_info($elementType);
  }

  public function buildContent(FieldFormElement $element, $view_mode = 'full') {
    $form = array();
    $settings = get_object_vars($element);
    foreach ($settings as $name => $setting) {
      if (!$element->hasChild($name)) {
        $form['#'.$name] = $setting;
      }
    }
    return $form;
  }
}