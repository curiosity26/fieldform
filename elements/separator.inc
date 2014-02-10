<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 1/29/14
 * Time: 12:47 PM
 */

class SeparatorElementController extends FieldFormElementController {
  static public function settingsForm($form, &$form_state, FieldFormElementInterface $element = NULL, $op = NULL) {
    $form = parent::settingsForm($form, $form_state, $element, $op);
    unset($form['required'], $form['description'], $form['details']);
    return $form;
  }

  static public function buildElement(FieldFormElementInterface $element, $view_mode = 'full') {
    return array(
      '#name' => $element->elementType(),
      '#type' => 'hidden',
      '#value' => $element->name
    );
  }

  static public function submit(FieldFormElementInterface $element, &$form, &$form_state) {
    $form_state['form_separator'] = $element->_id;
    $form_state['rebuild'] = TRUE;
    $form_state['submitted'] = FALSE;
    $form_state['executed'] = FALSE;
  }
}

class SeparatorResultDataController extends FieldFormResultDataController {
  static public function render(FieldFormResultDataInterface $result, $view_mode = 'full') {
    return NULL;
  }
}