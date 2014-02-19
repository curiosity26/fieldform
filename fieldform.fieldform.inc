<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 1/18/14
 * Time: 2:31 PM
 */

function fieldform_fieldform_info() {
  $info = array();

  $info['textfield'] = array(
    'label' => t('Textfield')
  );

  $info['textarea'] = array(
    'label' => t('Textarea'),
    'element controller' => 'TextAreaElementController',
    'properties' => array(
      'default_value' => array(
        'type' => FIELDFORM_PROPERTY_TEXT
      ),
      'rows' => array(
        'type' => FIELDFORM_PROPERTY_STRING
      ),
      'cols' => array(
        'type' => FIELDFORM_PROPERTY_STRING
      )
    ),
    'file' => 'textarea.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  $info['email'] = array(
    'label' => t('Email'),
    'element controller' => 'EmailElementController',
    'file' => 'email.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  $info['select'] = array(
    'label' => t('Select'),
    'element controller' => 'OptionsElementController',
    'properties' => array(
      'options' => array(
        'title' => t('Values'),
        'type' => FIELDFORM_PROPERTY_TABLE
      ),
      'multiple' => array(
        'type' => FIELDFORM_PROPERTY_BOOL
      ),
      'default_value' => array(
        'type' => FIELDFORM_PROPERTY_SELECT
      )
    ),
    'file' => 'options.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  $info['radios'] = array(
    'label' => t('Radio Buttons (Multiple)'),
    'element controller' => 'OptionsElementController',
    'properties' => array(
      'options' => array(
        'type' => FIELDFORM_PROPERTY_TABLE,
      ),
      'default_value' => array(
        'type' => FIELDFORM_PROPERTY_RADIOS
      )
    ),
    'file' => 'options.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  $info['radio'] = array(
    'label' => t('Radio Button (Single)'),
    'properties' => array(
      'return_value' => array(
        'type' => FIELDFORM_PROPERTY_STRING
      ),
      'default_value' => array(
        'title' => t('Checked'),
        'type' => FIELDFORM_PROPERTY_BOOL
      )
    )
  );

  $info['checkboxes'] = array(
    'label' => t('Checkboxes (Multiple)'),
    'element controller' => 'CheckboxesElementController',
    'properties' => array(
      'options' => array(
        'type' => FIELDFORM_PROPERTY_TABLE
      ),
      'default_value' => array(
        'type' => FIELDFORM_PROPERTY_CHECKBOXES
      )
    ),
    'file' => 'checkboxes.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  $info['checkbox'] = array(
    'label' => t('Checkbox (Single)'),
    'properties' => array(
      'return_value' => array(
        'type' => FIELDFORM_PROPERTY_STRING
      ),
      'default_value' => array(
        'title' => t('Checked'),
        'type' => FIELDFORM_PROPERTY_BOOL
      )
    )
  );

  $info['fieldset'] = array(
    'label' => t('Fieldset'),
    'element controller' => 'FieldsetElementController',
    'properties' => array(
      'collapsible' => array(
        'type' => FIELDFORM_PROPERTY_BOOL,
        'default_value' => TRUE
      ),
      'collapsed' => array(
        'type' => FIELDFORM_PROPERTY_BOOL
      ),
      'default_value' => array(
        'type' => FIELDFORM_PROPERTY_NONE
      ),
      'required' => array(
        'type' => FIELDFORM_PROPERTY_NONE
      ),
      'description' => array(
        'type' => FIELDFORM_PROPERTY_NONE
      )
    ),
    'has children' => TRUE,
    'file' => 'fieldset.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  $info['markup'] = array(
    'label' => t('HTML Markup'),
    'element controller' => 'MarkupElementController',
    'properties' => array(
      'markup' => array(
        'title' => t('HTML Code'),
        'type' => FIELDFORM_PROPERTY_MARKUP
      ),
      'default_value' => array(
        'type' => FIELDFORM_PROPERTY_NONE
      )
    ),
    'file' => 'markup.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  $info['date'] = array(
    'label' => t('Date'),
    'element controller' => 'DateElementController',
    'properties' => array(
      'default_value' => array(
        'type' => FIELDFORM_PROPERTY_DATE
      )
    ),
    'file' => 'date.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  $info['file'] = array(
    'label' => t('File'),
    'element controller' => 'FileElementController',
    'properties' => array(
      'default_value' => array(
        'type' => FIELDFORM_PROPERTY_NONE
      ),
    ),
    'file' => 'file.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  $info['separator'] = array(
    'label' => t('Page Separator'),
    'element controller' => 'SeparatorElementController',
    'properties' => array(
      'default_value' => array(
        'type' => FIELDFORM_PROPERTY_NONE
      )
    ),
    'file' => 'separator.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  $info['nodefield'] = array(
    'label' => t('Node Field'),
    'element controller' => 'NodeFieldElementController',
    'properties' => array(
      'default_value' => array(
        'type' => FIELDFORM_PROPERTY_NONE
      )
    ),
    'file' => 'nodefield.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  return $info;
}

function fieldform_result_info() {
  $info = array();

  $info['default'] = array(
    'result class' => 'FieldFormResultData',
    'result controller' => 'FieldFormResultDataController',
    'module' => 'fieldform'
  );

  $info['email'] = array(
    'result controller' => 'EmailResultDataController',
    'file' => 'email.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  $info['separator'] = array(
    'result controller' => 'SeparatorResultDataController',
    'file' => 'separator.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  $info['file'] = array(
    'result controller' => 'FileResultDataController',
    'file' => 'file.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  $info['markup'] = array(
    'result controller' => 'MarkupResultDataController',
    'file' => 'markup.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  $info['date'] = array(
    'result controller' => 'DateResultDataController',
    'file' => 'date.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  $info['fieldset'] = array(
    'result controller' => 'FieldsetResultDataController',
    'file' => 'fieldset.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  $info['radios'] = array(
    'result controller' => 'SelectResultDataController',
    'file' => 'select.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  $info['checkboxes'] = array(
    'result controller' => 'OptionsResultDataController',
    'file' => 'options.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  $info['radios'] = array(
    'result controller' => 'RadiosResultDataController',
    'file' => 'radios.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  $info['select'] = array(
    'result controller' => 'OptionsResultDataController',
    'file' => 'options.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  $info['radio'] = array(
    'result controller' => 'SingleOptionResultDataController',
    'file' => 'singleoption.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  $info['checkbox'] = array(
    'result controller' => 'SingleOptionResultDataController',
    'file' => 'singleoption.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  $info['textarea'] = array(
    'result controller' => 'TextAreaResultDataController',
    'file' => 'textarea.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  $info['nodefield'] = array(
    'result controller' => 'NodeFieldResultDataController',
    'file' => 'nodefield.inc',
    'file path' => drupal_get_path('module', 'fieldform').'/elements'
  );

  return $info;
}