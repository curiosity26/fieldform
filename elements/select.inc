<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 1/24/14
 * Time: 9:50 AM
 */

module_load_include('inc', 'fieldform', 'elements/options');

class SelectElementController extends OptionsElementController {
  static public function settingsForm($form, &$form_state, FieldFormElement $element = NULL, $op = NULL) {
    $values = !empty($form_state['input']) ? $form_state['input'] : get_object_vars($element);
    $multiple = 0;

    if (isset($values['attributes']) && isset($values['attributes']['multiple'])) {
      $multiple = $values['attributes']['multiple'];
      unset($values['attributes']['multiple']);
    }

    $form = parent::settingsForm($form, $form_state, $element, $op);

    $form['default_value']['#prefix'] = '<div id="default-value-element-ajax">';
    $form['default_value']['#suffix'] = '</div>';
    $form['default_value']['#options'] = $form['required']['#default_value'] == 1 ? array() : array('--' => t('None'));

    unset($form['default_value']['#default_value']);

    $form['multiple']['#default_value'] = $multiple;
    if ($form['multiple']['#default_value'] == 1) {
      $form['default_value']['#attributes']['multiple'] = 'multiple';
    }

    $form['options']['#options'] = is_array($form['options']['#options']) ?
      $form['options']['#options'] : array();

    foreach ($form['options']['#options'] as $option) {
      if (isset($option['name'])) {
        $form['default_value']['#options'][$option['name']] = t($option['value']);
      }
    }

    $form['options']['#push_values'] = array(
      '#default-value-element-ajax' => array(
        'replace' => array('form_ajax_options_change')
      )
    );

    $form['required']['#ajax'] = array(
      'wrapper' => '#default-value-element-ajax',
      'callback' => 'form_ajax_required_toggle',
      'method' => 'replace'
    );
    $form['multiple']['#ajax'] = array(
      'wrapper' => '#default-value-element-ajax',
      'callback' => 'form_ajax_multiple_toggle',
      'method' => 'replace'
    );
    return $form;
  }

  static public function settingsSubmitForm(&$form, &$form_state, $entity_type, $entity, $element_type, $element) {
    $form_state['input']['attributes']['multiple'] = ($form_state['input']['multiple'] == 1);
    parent::settingsSubmitForm($form, $form_state, $entity_type, $entity, $element_type, $element);
  }
}