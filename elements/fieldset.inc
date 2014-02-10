<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 1/24/14
 * Time: 5:39 PM
 */

class FieldsetElementController extends FieldFormElementController {
  static public function settingsForm($form, &$form_state, FieldFormElement $element = NULL, $op = NULL) {
    $form = parent::settingsForm($form, $form_state, $element, $op);
    unset($form['required']);
    unset($form['description']);
    unset($form['details']['title_display']);
    return $form;
  }
}

class FieldsetResultDataController extends FieldFormResultDataController {
  static public function processResult(FieldFormResultDataInterface $result, FieldFormResultEntity $entity,
                                       FieldFormElementInterface $element, $value) {
    return $element->title;
  }
}