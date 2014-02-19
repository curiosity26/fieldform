<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 2/7/14
 * Time: 8:39 AM
 */

class MarkupElementController extends FieldFormElementController {
  static public function settingsForm($form, &$form_state, FieldFormElementInterface $element = NULL, $op = NULL) {
    $form = parent::settingsForm($form, $form_state, $element, $op);
    unset($form['required']);
    $form['markup']['#default_value'] = isset($element->markup['value']) ? $element->markup['value'] : NULL;
    $form['markup']['#format'] = isset($element->markup['format']) ? $element->markup['format'] : NULL;
    return $form;
  }

  static public function buildElement(FieldFormElementInterface $element, FieldFormEntity $entity,
                                      $form = array(), &$form_state = array()) {
    $markup = parent::buildElement($element, $entity, $form, $form_state);
    $markup['#markup'] = $markup['#markup']['value'];
    return $markup;
  }
}

class MarkupResultDataController extends FieldFormResultDataController {
  static public function processResult(FieldFormResultDataInterface $result, FieldFormResultEntity $entity,
                                       FieldFormElementInterface $element, $value) {
    return $element->markup['value'];
  }

  static public function render(FieldFormResultDataInterface $result, $view_mode = 'full') {
    $output = parent::render($result, $view_mode);
    $output['#title_display'] = 'hidden';
    return $output;
  }
}