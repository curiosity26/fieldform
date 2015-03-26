<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 1/16/14
 * Time: 2:32 PM
 */

function fieldform_form($form, &$form_state, Entity $entity, $op, $entity_type) {
  $info = $entity->entityInfo();

  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#default_value' => !empty($form_state['values']['label']) ? $form_state['values']['label'] : $entity->label(),
    '#required' => TRUE
  );

  $form['name'] = array(
    '#type' => 'machine_name',
    '#machine_name' => array(
      'source' => array('label'),
      'exists' => 'fieldform_name_exists'
    )
  );

  $form['settings'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#title' => t('Form Settings')
  );

  $form['settings']['completed_behavior'] = array(
    '#type' => 'radios',
    '#title' => t('Completed Form Behavior'),
    '#options' => array(
      'message' => t('Display a message'),
      'url' => t('Redirect to another URL')
    ),
    '#default_value' => !empty($form_state['values']['settings']) &&
      isset($form_state['values']['settings']['completed_behavior']) ?
        $form_state['values']['settings']['completed_behavior'] : $entity->settings['completed_behavior'],
    '#description' => t('What action you would like to take once the form is completed.')
  );

  $form['settings']['completed_behavior_message_container'] = array(
    '#type' => 'container',
    '#tree' => FALSE,
    '#states' => array(
      'visible' => array(
        ':input[name="settings[completed_behavior]"]' => array('value' => 'message')
      )
    )
  );

  $form['settings']['completed_behavior_message_container']['completed_behavior_message'] = array(
    '#type' => 'text_format',
    '#title' => t('Message'),
    '#default_value' => isset($form_state['values']['settings']['completed_behavior_message']) ?
        $form_state['values']['settings']['completed_behavior_message']['value']
        : $entity->settings['completed_behavior_message']['value'],
    '#parents' => array('settings', 'completed_behavior_message')
  );

  $format = isset($form_state['values']['settings']['completed_behavior_message']) ?
    $form_state['values']['settings']['completed_behavior_message']['format']
    : $entity->settings['completed_behavior_message']['format'];

  if (isset($format)) {
    $form['settings']['completed_behavior_message_container']['completed_behavior_message']['#format'] = $format;
  }

  $form['settings']['completed_behavior_url'] = array(
    '#type' => 'textfield',
    '#title' => t('URL or Path'),
    '#default_value' => isset($form_state['values']['settings']['completed_behavior_url']) ?
        $form_state['values']['settings']['completed_behavior_url'] : $entity->settings['completed_behavior_url'],
    '#states' => array(
      'visible' => array(
        ':input[name="settings[completed_behavior]"]' => array('value' => 'url')
      )
    )
  );

  $form['settings']['submit_button_text'] = array(
    '#type' => 'textfield',
    '#title' => t('Submit Button Text'),
    '#default_value' => !empty($form_state['values']['settings']['submit_button_text']) ?
        $form_state['values']['settings']['submit_button_text'] : $entity->settings['submit_button_text']
  );

  $form['settings']['continue_button_text'] = array(
    '#type' => 'textfield',
    '#title' => t('Continue Button Text'),
    '#default_value' => !empty($form_state['values']['settings']['continue_button_text']) ?
        $form_state['values']['settings']['continue_button_text'] : $entity->settings['continue_button_text']
  );

  $form['settings']['back_button_text'] = array(
    '#type' => 'textfield',
    '#title' => t('Back Button Text'),
    '#default_value' => !empty($form_state['values']['settings']['back_button_text']) ?
        $form_state['values']['settings']['back_button_text'] : $entity->settings['back_button_text']
  );

  $form += array(
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#suffix' => l('cancel', check_url($info['admin ui']['path'])),
      '#weight' => 100
    )
  );

  $form['#submit'] = array('fieldform_form_submit');

  return $form;
}

function fieldform_form_submit($form, &$form_state) {
  $entity_type = $form_state['entity_type'];
  $entity = $form_state[$entity_type];
  $values = $form_state['values'];
  unset($values['form_id'], $values['form_build_id'], $values['form_build_token'], $values['op']);
  foreach ($values as $key => $value) {
    $entity->$key = $value;
  }
  entity_save($entity_type, $entity);
  $form_state['redirect'] = $entity->entityInfo()['admin ui']['path'];
  if ($form_state['build_info']['args'][1] == 'add') {
    $form_state['redirect'] .= '/manage/'.$entity->identifier();
  }
  drupal_redirect_form($form_state);
}

function fieldform_name_exists($value, $element, $form_state) {
  list($entity) = $form_state['build_info']['args'];
  $query = db_select('fieldform', 'f')->fields('f', array('name', 'fid'))->condition('name', $value);
  $result = $query->execute();
  if ($result->rowCount() > 0) {
    $row = $result->fetchAssoc();
    return !($entity->name === $value && $entity->fid === $row['fid']);
  }
  return FALSE;
}

function fieldform_elements_list($form, &$form_state, $entity) {
  $form['#fid']  = $form_state['fid'] = $entity->identifier();
  $values = isset($form_state['input']) ? $form_state['input'] : array();
  $elements = db_select('fieldform_elements', 'e', array('fetch' => PDO::FETCH_ASSOC))
    ->fields('e')->where('e.fid = :fid and e.pid = 0', array('fid' => $entity->identifier()))
    ->execute();

  $form['#element_base_uri'] = $entity->entityInfo()['admin ui']['path'];

  $form['elements'] = array(
    '#tree' => TRUE
  );
  _fieldform_element_form_row($form, $elements);

  $form['elements']['add_new_element']['title'] = array(
    '#type' => 'textfield',
    '#size' => 20,
    '#default_value' => isset($values['elements']['add_new_element']) ?
        $values['elements']['add_new_element']['title'] : NULL
  );

  $form['elements']['add_new_element']['name'] = array(
    '#type' => 'machine_name',
    '#machine_name' => array(
      'source' => array('elements', 'add_new_element', 'title'),
      'exists' => 'fieldform_elements_name_exists'
    ),
    '#required' => FALSE
  );

  $types = array();
  foreach (fieldform_get_info() as $type => $info) {
    $types[$type] = $info['label'];
  }

  $form['elements']['add_new_element']['type'] = array(
    '#type' => 'select',
    '#options' => $types,
    '#default_value' => isset($values['elements']['add_new_element']['type']) ?
        $values['elements']['add_new_element']['type'] : NULL,
    '#ajax' => array(
      'callback' => 'fieldform_type_select_ajax'
    )
  );

  $form['elements']['add_new_element']['id'] = array(
    '#type' => 'hidden',
    '#value' => 'add_new_element'
  );

  $form['elements']['add_new_element']['pid'] = array(
    '#type' => 'hidden',
    '#value' => ''
  );

  $form['elements']['add_new_element']['weight'] = array(
    '#type' => 'hidden',
    '#value' => 0
  );

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration')
  );

  return $form;
}

function _fieldform_element_form_row(&$form, $elements, $pid = 0, $depth = 0) {
  $base_url = $form['#element_base_uri'];

  $temp = array();
  if (!empty($elements)) {
    foreach ($elements as $element) {
      if (!is_object($element) || $element->_id === 'form_complete') {
        continue;
      }

      $temp['title_label'] = array(
        '#markup' => $element['title']
      );

      $temp['title'] = array(
        '#type' => 'hidden',
        '#value' => $element['title']
      );

      $temp['name_label'] = array(
        '#markup' => $element['name']
      );

      $temp['name'] = array(
        '#type' => 'hidden',
        '#value' => $element['name']
      );

      $temp['type_label'] = array(
        '#markup' => $element['type']
      );

      $temp['type'] = array(
        '#type' => 'hidden',
        '#value' => $element['elementType']
      );

      /*$temp['id'] = array(
        '#type' => 'hidden',
        '#value' => $element['id']
      );*/

      $temp['eid'] = array(
        '#type' => 'hidden',
        '#value' => $element['eid']
      );

      $temp['weight'] = array(
        '#type' => 'weight',
        '#value' => is_numeric($element['weight']) ? $element['weight'] : 0,
        '#delta' => 100
      );

      $temp['pid'] = array(
        '#type' => 'hidden',
        '#value' => $element['pid']
      );

      $temp['operations']['edit'] = array(
        '#markup' => l('edit', $base_url.'/manage/'.$form['#fid'].'/elements/'.$element['name'].'/edit')
      );

      $temp['operations']['delete'] = array(
        '#markup' => l('delete', $base_url.'/manage/'.$form['#fid'].'/elements/'.$element['name'].'/delete')
      );

      $temp['#depth'] = $depth;

      $form['elements'][$element['name']] = $temp;

      $children = $elements = db_select('fieldform_elements', 'e',
        array('fetch' => PDO::FETCH_ASSOC))
        ->fields('e')->where('e.fid = :fid and e.pid = :pid',
          array('fid' => $form['#fid'], 'pid' => $pid))
        ->execute();

      if ($element['pid'] !== $pid && !empty($children)) {
        _fieldform_element_form_row($form, $children, $element['pid'], $depth + 1);
      }
    }
  }
}

function fieldform_elements_name_exists($value, $element, $form_state) {
  $entity = $form_state['fieldform'];
  return $entity->findElementByName($value) !== FALSE;
}

function fieldform_type_select_ajax($form, $form_state) {
  $element = $form_state['triggering_element'];
  $value = $element['#value'];
  $info = fieldform_get_info($value);
  if ($info['has children'] === TRUE) {
    return array(
      '#type' => 'ajax',
      '#commands' => array(
        ajax_command_invoke("tr.add-new-element", "removeClass", array("tabledrag-leaf"))
      )
    );
  }
  else {
    return array(
      '#type' => 'ajax',
      '#commands' => array(
        ajax_command_invoke("tr.add-new-element", "addClass", array("tabledrag-leaf"))
      )
    );
  }
}

function fieldform_elements_list_submit($form, &$form_state) {
  $form_state['redirect'] = $form['#element_base_uri'].'/manage/'.$form['#fid'].'/elements';
  $values = $form_state['input'];
  $elements = $values['elements'];

  foreach ($elements as $name => $element) {
    if ($name === 'add_new_element') {
      drupal_write_record('fieldform_elements', $element);
    }
    else {
      drupal_write_record('fieldform_elements', $element, array('eid'));
    }
  }

  drupal_redirect_form($form_state);
}