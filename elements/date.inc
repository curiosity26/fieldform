<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 2/7/14
 * Time: 11:47 AM
 */

class DateElementController extends FieldFormElementController {
  static public function settingsForm($form, &$form_state, FieldFormElementInterface $element = NULL, $op = NULL) {
    $form = parent::settingsForm($form, $form_state, $element, $op);
    $form['current_date'] = array(
      '#type' => 'checkbox',
      '#title' => t('Always use the current date as default value'),
      '#default_value' => isset($element->current_date) ? $element->current_date : FALSE,
      '#weight' => $form['default_value']['#weight'] + 0.01
    );
    unset($form['required']);
    return $form;
  }

  static public function buildElement(FieldFormElementInterface $element) {
    $output = parent::buildElement($element);
    if (isset($element->current_date) && $element->current_date == TRUE) {
      $output['#default_value'] = array(
        'month' => date('n'),
        'day' => date('j'),
        'year' => date('Y')
      );
    }
    return $output;
  }
}

class DateResultDataController extends FieldFormResultDataController {
  static public function processResult(FieldFormResultDataInterface $result, FieldFormResultEntity $entity,
                                       FieldFormElementInterface $element, $value) {
    $time = strtotime($value['year'].'-'.$value['month'].'-'.$value['day']);
    return format_date($time, 'medium');
  }
}