<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 1/31/14
 * Time: 3:04 PM
 */

/* Field */

function fieldform_field_info() {
  return array(
    'fieldform' => array(
      'label' => t('Fieldform Form'),
      'description' => t('A form entity that can be attached to various other entities'),
      'settings' => array(
        'cardinality' => 1
      ),
      'default_widget' => 'fieldform_field_widget',
      'default_formatter' => 'fieldform_field_formatter'
    )
  );
}

function fieldform_field_is_empty($item, $field) {
  if ($field['type'] === 'fieldform' && empty($item['fieldform_form']) && (string) $item['fieldform_form'] !== '--') {
    return TRUE;
  }
  return FALSE;
}

function fieldform_form_field_ui_field_edit_form_alter(&$form, &$form_state, $form_id) {
  if ($form['#field']['type'] == 'fieldform') {
    $form['field']['cardinality']['#default_value'] = 1;
    $form['field']['cardinality']['#access'] = FALSE;
  }
}

/* Widget */

function fieldform_field_widget_info() {
  return array(
    'fieldform_field_widget' => array(
      'label' => t('Fieldform'),
      'field types' => array('fieldform'),
      'settings' => array(),
      'behaviors' => array(
        'multiple values' => FIELD_BEHAVIOR_DEFAULT,
        'default value' => FIELD_BEHAVIOR_DEFAULT,
      )
    )
  );
}

function fieldform_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {
  $entities = fieldform_load_multiple();
  $fieldforms = array('--' => t('-- Disable Form --'));

  foreach ($entities as $entity) {
    $fieldforms[$entity->identifier()] = $entity->label();
  }

  $element['fieldform_form'] = array(
    '#type' => 'select',
    '#title' => t('Field Form'),
    '#options' => $fieldforms,
    '#default_value' => isset($items[$delta]) ? $items[$delta] : '',
  );

  return $element;
}


/* Formatter */

function fieldform_field_formatter_info() {
  return array(
    'fieldform_field_formatter' => array(
      'label' => t('Fieldform Formatter'),
      'field types' => array('fieldform')
    )
  );
}

function fieldform_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  $element = array();
  if ($display['type'] === 'fieldform_field_formatter') {
    if (isset($items[0]['fieldform_form']) && $items[0]['fieldform_form'] !== '--') {
      $fieldform = fieldform_load($items[0]['fieldform_form']);
      if (fieldform_result_access('view', $fieldform)) {
        $element[0] = entity_view($fieldform->entityType(), array($fieldform));
      }
    }
  }

  return $element;
}