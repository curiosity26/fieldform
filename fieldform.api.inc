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
      'element class' => 'FieldFormElement',
      'module' => 'fieldform',
      'element controller' => 'FieldFormElementController',
      'element ui' => array(
        'full' => array(
          'controller' => 'FieldFormElementUIController',
          'file' => 'fieldform.ui',
          'file path' => drupal_get_path('module', 'fieldform')
        )
      ),
      'file' => 'fieldform.inc',
      'file path' => drupal_get_path('module', 'fieldform')
    )
  );
}