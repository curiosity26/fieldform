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

  static public function buildElement(FieldFormElementInterface $element, FieldFormEntity $entity,
                                      $form = array(), &$form_state = array()) {
    $output = parent::buildElement($element, $entity, $form, $form_state);
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
  static public function buildElement(FieldFormResultDataInterface $result, &$element, $default_value = NULL) {
    if (!isset($default_value) || !is_array($default_value)) {
      $value = isset($default_value) ? $default_value : $result->value();
      if (!is_array($value)) {
        $format = variable_get('date_format_medium', 'D, m/d/Y - H:i');
        $time = DateTime::createFromFormat($format, $value, new DateTimeZone(drupal_get_user_timezone()))
          ->getTimestamp();
        $value = array(
          'month' => date('n', $time),
          'day' => date('j', $time),
          'year' => date('Y', $time)
        );
      }
      $element['#default_value'] = $value;
    }
  }

  static public function preRender(FieldFormResultDataInterface $result, &$vars) {
    $value = $result->value();
    $time = strtotime($value['year'].'-'.$value['month'].'-'.$value['day']);
    $vars['content'] = format_date($time, 'medium');
  }
}