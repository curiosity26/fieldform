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

  $info['select'] = array(
    'label' => t('Select'),
    'properties' => array(
      'options' => array(
        'type' => FIELDFORM_PROPERTY_TABLE
      ),
      'multiple' => array(
        'type' => FIELDFORM_PROPERTY_BOOL
      ),
      'default_value' => array(
        'type' => FIELDFORM_PROPERTY_SELECT
      )
    )
  );

  $info['radios'] = array(
    'label' => t('Radio Buttons (Multiple)'),
    'properties' => array(
      'options' => array(
        'type' => FIELDFORM_PROPERTY_TABLE,
      ),
      'default_value' => array(
        'type' => FIELDFORM_PROPERTY_RADIOS
      )
    )
  );

  $info['radio'] = array(
    'label' => t('Radio Button (Single)'),
    'properties' => array(
      'return_value' => array(
        'type' => FIELDFORM_PROPERTY_STRING
      ),
      'checked' => array(
        'type' => FIELDFORM_PROPERTY_BOOL
      ),
      'default_value' => array(
        'type' => FIELDFORM_PROPERTY_NONE
      )
    )
  );

  $info['checkboxes'] = array(
    'label' => t('Checkboxes (Multiple)'),
    'properties' => array(
      'options' => array(
        'type' => FIELDFORM_PROPERTY_TABLE
      ),
      'default_value' => array(
        'type' => FIELDFORM_PROPERTY_CHECKBOXES
      )
    )
  );

  $info['checkbox'] = array(
    'label' => t('Checkbox (Single)'),
    'properties' => array(
      'return_value' => array(
        'type' => FIELDFORM_PROPERTY_STRING
      ),
      'checked' => array(
        'type' => FIELDFORM_PROPERTY_BOOL
      ),
      'default_value' => array(
        'type' => FIELDFORM_PROPERTY_NONE
      )
    )
  );

  $info['markup'] = array(
    'label' => t('HTML Markup'),
    'properties' => array(
      'markup' => array(
        'title' => t('HTML Code'),
        'type' => FIELDFORM_PROPERTY_MARKUP
      ),
      'default_value' => array(
        'type' => FIELDFORM_PROPERTY_NONE
      )
    )
  );

  $info['date'] = array(
    'label' => t('Date'),
    'properties' => array(
      'default_value' => array(
        'type' => FIELDFORM_PROPERTY_DATE
      ),
      'use_today' => array(
        'title' => t('Use Current Date'),
        'type' => FIELDFORM_PROPERTY_BOOL
      )
    )
  );

  $info['file'] = array(
    'label' => t('File'),
    'properties' => array(
      'default_value' => array(
        'type' => FIELDFORM_PROPERTY_NONE
      ),
    )
  );

  return $info;
}