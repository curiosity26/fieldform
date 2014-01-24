<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 1/24/14
 * Time: 4:07 PM
 */

class OptionsElementController extends FieldFormElementController {
  public function requiredToggle($form, $form_state) {
    $wrapper = $form_state['triggering_element']['#ajax']['wrapper'];
    $method = $form_state['triggering_element']['#ajax']['method'];
    $func = 'ajax_command_'.$method;
    $commands = array();
    if ($form_state['input']['required'] != 1) {
      $options = array('--' => t('None')) + $form['default_value']['#options'];
      array_filter($options);
      $form['default_value']['#options'] = $options;
    }
    else {
      unset($form['default_value']['#options']['--']);
    }

    if ($form_state['input']['multiple'] == 1) {
      $form['default_value']['#attributes']['multiple'] = 'multiple';
    }

    unset($form['default_value']['#printed']);
    $commands[] = $func($wrapper, render($form['default_value']));
    return $commands;
  }

  public function multipleToggle($form, $form_state) {
    $wrapper = $form_state['triggering_element']['#ajax']['wrapper'];
    $selector = $wrapper.' .form-'.$this->elementType;
    if ($form_state['input']['multiple'] == 1) {
      $commands = array(
        ajax_command_invoke($selector, 'attr', array('multiple', 'multiple'))
      );
    }
    else {
      $commands = array(
        ajax_command_invoke($selector, 'removeAttr', array('multiple'))
      );
    }
    return $commands;
  }
}
