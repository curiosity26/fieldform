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
  $form_state['fieldform'] = $entity;
  $values = isset($form_state['input']) ? $form_state['input'] : array();
  $elements = $entity->elements;
  $mem_usage = memory_get_usage();
  $form['elements'] = array(
    '#tree' => TRUE
  );
  _fieldform_element_form_row($form, $elements, $entity);

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

  $form['elements']['add_new_element']['parent'] = array(
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

function _fieldform_element_form_row(&$form, $elements, $entity) {
  $base_url = $entity->entityInfo()['admin ui']['path'];
  $temp = array();
  if (!empty($elements)) {
    foreach ($elements as $element) {
      if (is_string($element)) {
        $element = unserialize($element);
      }
      if (!is_object($element) || $element->_id === 'form_complete') {
        continue;
      }

      $parents = $element->parents;
      array_pop($parents);
      $parent = $entity->findElementByName(array_pop($parents));

      $temp['title_label'] = array(
        '#markup' => $element->title
      );

      $temp['title'] = array(
        '#type' => 'hidden',
        '#value' => $element->title
      );

      $temp['name_label'] = array(
        '#markup' => $element->name
      );

      $temp['name'] = array(
        '#type' => 'hidden',
        '#value' => $element->name
      );

      $temp['type_label'] = array(
        '#markup' => $element->type
      );

      $temp['type'] = array(
        '#type' => 'hidden',
        '#value' => $element->elementType()
      );

      $temp['id'] = array(
        '#type' => 'hidden',
        '#value' => $element->_id
      );

      $temp['weight'] = array(
        '#type' => 'weight',
        '#value' => is_numeric($element->weight) ? $element->weight : 0,
        '#delta' => 100
      );

      $temp['parent'] = array(
        '#type' => 'hidden',
        '#value' => $parent !== FALSE ? $parent->_id : ''
      );

      $temp['depth'] = array(
        '#type' => 'hidden',
        '#value'=> count($element->parents) - 1
      );

      $temp['operations']['edit'] = array(
        '#markup' => l('edit', $base_url.'/manage/'.$entity->identifier().'/elements/'.$element->name.'/edit')
      );

      $temp['operations']['delete'] = array(
        '#markup' => l('delete', $base_url.'/manage/'.$entity->identifier().'/elements/'.$element->name.'/delete')
      );

      $form['elements'][$element->name] = $temp;

      if ($element->hasChildren()) {
        _fieldform_element_form_row($form, $element->children, $entity);
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
  $entity = $form_state['fieldform'];
  $form_state['redirect'] = $entity->entityInfo()['admin ui']['path'].'/manage/'.$entity->identifier().'/elements';
  $values = $form_state['input'];
  $elements = $values['elements'];
  $process = array();
  $old_elements = $entity->elements;

  foreach ($elements as $name => $value) {
    if ($name !== 'add_new_element') {
      $element = $entity->findElementByID($value['id']);
      if (!$element) {
        $entity->elements = $old_elements;
        throw new DrupalUpdateException();
      }
      $arr = get_object_vars($element);
      unset($arr['parents'], $arr['children'], $arr['weight']); // Ensure that these are made new
      $process[] = array_merge($arr, $value);
    }
    elseif($value['name'] !== '') {
      $process[] = $value;
      $form_state['redirect'] .= '/'.$value['name'].'/edit';
    }
  }

  $entity->elements = array();

  try {
    _fieldform_process_elements($entity, $process);
    entity_save($entity->entityType(), $entity);
  }
  catch(Exception $e) {
    $entity->elements = $old_elements;
    throw $e;
  }

  drupal_redirect_form($form_state);
}

function _fieldform_process_elements(FieldFormEntity $entity, $elements) {
  while($current = array_shift($elements)) {
    if (isset($current['parent']) && $current['parent'] != NULL && $current['parent'] !== 'add_new_element') {
      $parent_id = $current['parent'];
      unset($current['parent']);
      $parent = $entity->findElementByID($parent_id);
      if (!$parent) {
        _fieldform_process_elements($entity, $elements);
        $parent = $entity->findElementByID($parent_id);
      }
      if ($parent) {
        $current['parents'] = array_merge($parent->parents, array($current['name']));
        $parent->addElement($current);
      }
    }
    elseif (isset($current['name']) && isset($current['type'])) {
      $id = $current['id'];
      $element = _fieldform_update_entity($entity, $current);
      if ($element && $id === 'add_new_element') {
        foreach ($elements as &$process) {
          if ($process['parent'] === 'add_new_element') {
            $process['parent'] = $element->_id;
          }
        }
      }
    }
  }
}

function _fieldform_update_entity(FieldFormEntity $entity, $element) {
  unset($element['id']);
  $element = fieldform_element_create($element['type'], $element);
  if ($element) {
    $success = elements_add($entity, $element);
    if ($success === FALSE) {
      drupal_set_message('Failed to add element', 'error');
    }
    return $success;
  }
  return FALSE;
}