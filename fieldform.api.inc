<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 1/16/14
 * Time: 1:15 PM
 */

function hook_fieldform_info() {
  return array(
    'textfield' => array(
      'label' => t('Textfield'),
      'element class' => 'FieldFormElement',
      'element controller' => 'FieldFormElementController',
      'module' => 'fieldform',
      'properties' => array(
        'default_value' => array(
          'name' => 'default_value',
          'title' => t('Default Value'),
          'type' => FIELDFORM_PROPERTY_STRING
        )
      ),
      'has children' => FALSE, // Can the element have child elements, i.e. Fieldset
      // The following is optional. Overrides $controller->[op]Element (where [op] is add, update, etc)
      // and default system [op] function
      'element add callback' => 'callback_function_name',
      'element update callback' => 'callback_function_name',
      'element delete callback' => 'callback_function_name',
      // The following are optional file paths to be used for the element classes and callbacks.
      'file' => 'fieldform.inc',
      'file path' => drupal_get_path('module', 'fieldform')
    )
  );
}