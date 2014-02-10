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

  static public function buildElement(FieldFormElementInterface $element) {
    $markup = parent::buildElement($element);
    $markup['#markup'] = $markup['#markup']['value'];
    return $markup;
  }
}

class MarkupResultDataController extends FieldFormResultDataController {
  static public function processResult(FieldFormResultDataInterface $result, FieldFormResultEntity $entity,
                                       FieldFormElementInterface $element, $value) {
    return $value['value'];
  }

  static public function preRender(FieldFormResultDataInterface $result, &$vars) {
    $vars['title'] = NULL;
    $vars['content'] = $result->value();
  }

}