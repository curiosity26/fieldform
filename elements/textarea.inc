<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 1/20/14
 * Time: 11:06 AM
 */

class TextAreaElementController extends FieldFormElementController {
  static public function settingsForm($form, &$form_state, FieldFormElement $element = NULL, $op = NULL) {
    $form = parent::settingsForm($form, $form_state, $element, $op);
    $form['rows']['#size'] = 4;
    $form['cols']['#size'] = 4;

    return $form;
  }
}

class TextAreaResultDataController extends FieldFormResultDataController {
  static public function buildElement(FieldFormResultDataInterface $result, &$element, $default_value = NULL) {
    $value = isset($default_value) ? $default_value : $result->value();
    $value = preg_replace('#<br\s*/?>#i', "\n", $value);
    parent::buildElement($result, $element, $value);
  }

  static public function processResult(FieldFormResultDataInterface $result, FieldFormResultEntity $entity,
                                       FieldFormElementInterface $element, $value) {

    return preg_replace('/(\r\n|\r|\n)+/', '<br>', $value); // This is 2014. Use HTML5 or get lost
  }
}