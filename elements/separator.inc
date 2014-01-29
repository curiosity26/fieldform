<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 1/29/14
 * Time: 12:47 PM
 */

class SeparatorElementController extends FieldFormElementController {
  static public function settingsForm($form, &$form_state, FieldFormElementInterface $element = NULL, $op = NULL) {
    unset($form['required'], $form['description'], $form['details']);
    return $form;
  }

  static public function buildElement(FieldFormElementInterface $element, $view_mode = 'full') {
    return array(
      '#type' => 'hidden',
    );
  }
}