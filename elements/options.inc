<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 1/24/14
 * Time: 4:07 PM
 */

class OptionsElementController extends FieldFormElementController {
  static public function settingsForm($form, &$form_state, FieldFormElement $element = NULL, $op = NULL) {
    $values = !empty($form_state['input']) ? $form_state['input'] : $element->toArray();
    $multiple = 0;

    if (isset($values['attributes']) && isset($values['attributes']['multiple'])) {
      $multiple = $values['attributes']['multiple'];
      unset($values['attributes']['multiple']);
    }

    $form = parent::settingsForm($form, $form_state, $element, $op);

    $form['options']['#options'] = is_array($form['options']['#options']) ?
      $form['options']['#options'] : array();

    $form['options']['#push_values'] = array(
      '#default-value-element-ajax' => array(
        'replace' => array('form_ajax_options_change')
      )
    );

    $options = isset($values['options']) ? (array)$values['options'] : array();

    $form['default_value']['#prefix'] = '<div id="default-value-element-ajax">';
    $form['default_value']['#suffix'] = '</div>';
    $form['default_value']['#options'] = $form['required']['#default_value'] != 1 ?
      array('--' => t('None')) + $options : $options;

    if (!empty($form['multiple'])) {
      $form['multiple']['#default_value'] = $multiple;

      if ($form['multiple']['#default_value'] == 1) {
        $form['default_value']['#attributes']['multiple'] = 'multiple';
      }

      $form['multiple']['#ajax'] = array(
        'wrapper' => '#default-value-element-ajax',
        'callback' => 'form_ajax_multiple_toggle',
        'method' => 'replace'
      );
    }

    $form['required']['#ajax'] = array(
      'wrapper' => '#default-value-element-ajax',
      'callback' => 'form_ajax_required_toggle',
      'method' => 'replace'
    );

    return $form;
  }

  static public function settingsSubmitForm(&$form, &$form_state, $entity_type, $entity, $element_type, $element) {
    if (isset($form_state['input']['attributes']['multiple'])) {
      $form_state['input']['attributes']['multiple'] = ($form_state['input']['multiple'] == 1);
    }

    $form_state['values']['options'] = $form_state['values']['options']['values'];

    parent::settingsSubmitForm($form, $form_state, $entity_type, $entity, $element_type, $element);
  }

  public function requiredToggle($form, $form_state) {
    $wrapper = $form_state['triggering_element']['#ajax']['wrapper'];
    $method = $form_state['triggering_element']['#ajax']['method'];
    $func = 'ajax_command_'.$method;
    $commands = array();
    $values = drupal_array_get_nested_value($form_state['values'], $form['options']['#parents']);
    $values = isset($values['values']) ? array_filter($values['values']) : array();
    if ($form_state['input']['required'] != 1) {
      $options = array('--' => t('None'));
      $options += $values;
      $form['default_value']['#options'] = $options;
    }
    else {
      $form['default_value']['#options'] = $values;
    }

    if ($form_state['input']['multiple'] == 1) {
      $form['default_value']['#attributes']['multiple'] = 'multiple';
    }

    foreach(element_children($form['default_value']) as $child) {
      unset($form['default_value'][$child]);
    }
    unset($form_state['complete_form']['default_value']);
    form_builder($form['#form_id'], $form['default_value'], $form_state);
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
    unset($form['default_value']['#printed']);
    return render($form['default_value']);
  }

  static public function validate(FieldFormElementInterface $element, &$form, &$form_state) {
    $field = drupal_array_get_nested_value($form, $element->parents);
    dpm($field['#value'], "Value {$element->name}");
    dpm($field['#options'], "Options {$element->name}");
  }
}

class OptionsResultDataController extends FieldFormResultDataController {
  static public function processResult(FieldFormResultDataInterface $result, FieldFormResultEntity $entity,
                                       FieldFormElementInterface $element, $value) {
    if (is_array($value)) {
      $ret = array();
      foreach($value as $option) {
        if ($option != FALSE) {
          $ret[] = $element->options[$option];
        }
      }
      return $ret;
    }
    return isset($element->options[$value]) ? $element->options[$value] : NULL;
  }

  static public function buildElement(FieldFormResultDataInterface $result, &$element, $default_value = NULL) {
    if (!isset($default_value) && isset($element['#options'])) {
      $value = $result->value();
      if (is_array($value)) {
        if (!is_array($default_value)) {
          $default_value = array($default_value);
        }
        foreach ($value as $option) {
          if (($key = array_search($option, $element['#options'])) !== FALSE) {
            $default_value[] = $key;
          }
        }
      }
      else {
        foreach ($element['#options'] as $k => $o) {
          if ($o === $value) {
            $default_value = $k;
            break;
          }
        }
      }
      $element['#default_value'] = $default_value;
    }
    unset($element['#value']);
  }
}