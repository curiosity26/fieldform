<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 3/28/14
 * Time: 10:17 AM
 */

function fieldform_rules_event_info() {
  $info = array();

  $info['fieldform_completed'] = array(
    'label' => t('Field Form Completed'),
    'type' => 'fieldform_result',
    'description' => t('The field form has been completed by the user'),
    'group' => t('Field Form Result'),
    'variables' => array(
      'fieldform_result' => array(
        'type' => 'entity',
        'label' => t('Field Form Result')
      )
    )
  );

  return $info;
}