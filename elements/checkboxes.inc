<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 2/6/14
 * Time: 5:04 PM
 */

class CheckboxesElementController extends FieldFormElementController {
  static public function settingsForm($form, &$form_state, FieldFormElement $element = NULL, $op = NULL) {
    $form = parent::settingsForm($form, $form_state, $element, $op);
    $form['options']['#value'] = isset($form['options']['#value']['values']) ?
      $form['options']['#value']['values'] : array();

    $form['options']['#push_values'] = array(
      '#default-value-element-ajax' => array(
        'replace' => array('form_ajax_options_change')
      )
    );

    $form['default_value']['#options'] = array_filter($form['options']['#default_value']);
    $form['default_value']['#prefix'] = '<div id="default-value-element-ajax">';
    $form['default_value']['#suffix'] = '</div>';

    $form['required']['#ajax'] = array(
      'wrapper' => '#default-value-element-ajax',
      'callback' => 'form_ajax_required_toggle',
      'method' => 'replace'
    );

    return $form;
  }

  static public function settingsSubmitForm(&$form, &$form_state, $entity_type, $entity, $element_type, $element) {
    $form_state['values']['options'] = $form_state['values']['options']['values'];
    parent::settingsSubmitForm($form, $form_state, $entity_type, $entity, $element_type, $element);
  }

  public function optionsChange($form, $form_state, array $values = array()) {
    $data = $form['required']['#checked'] == TRUE ? array() : array('--' => t('None'));
    $data += $values;
    $data = array_filter($data);
    $form['default_value']['#options'] = $data;
    foreach(element_children($form['default_value']) as $child) {
      unset($form['default_value'][$child]);
    }
    unset($form_state['complete_form']['default_value']);
    form_builder($form['#form_id'], $form['default_value'], $form_state);
    return render($form['default_value']);
  }
}