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
      'source' => 'label',
    )
  );

  $form += array(
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#suffix' => l('cancel', check_url($info['admin ui']['path'])),
      '#weight' => 100
    )
  );

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
}

function fieldform_elements_list($form, &$form_state, $entity) {
  $form_state['fieldform'] = $entity;
  $values = $form_state['values'];
  $elements = $entity->elements;

  $form['elements']['#tree'] = TRUE;

  _fieldform_element_form_row($form['elements'], $elements, $entity);

  $form['elements']['add_new_element']['id'] = array(
    '#type' => 'hidden',
    '#value' => 'add_new_value'
  );

  $form['elements']['add_new_element']['title'] = array(
    '#type' => 'textfield',
    '#size' => 20,
    '#default_value' => $values['elements']['add_new_element']['title']
  );

  $form['elements']['add_new_element']['name'] = array(
    '#type' => 'machine_name',
    '#machine_name' => array(
      'source' => array('elements', 'add_new_element', 'title')
    )
  );

  $types = fieldform_get_info();

  $form['elements']['add_new_element']['type'] = array(
    '#type' => 'select',
    '#options' => array_keys($types),
    '#default_value' => $values['elements']['add_new_element']['type']
  );

  $form['elements']['add_new_element']['parents'] = array(
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
  foreach ($elements as $name => $element) {
    $pid = !empty($element['parents']) ? implode(':', $element['parents']) : NULL;

    $form[$name]['id'] = array(
      '#type' => 'hidden',
      '#value' => $pid.':'.$name
    );

    $form[$name]['title'] = array(
      '#markup' => $element['title']
    );

    $form[$name]['name'] = array(
      '#markup' => $element['name']
    );

    $form[$name]['type'] = array(
      '#markup' => $element['type']
    );

    $form[$name]['weight'] = array(
      '#type' => 'hidden',
      '#value' => is_numeric($element['weight']) ? $element['weight'] : 0
    );

    $pid = !empty($element['parents']) ? implode(':', $element['parents']) : NULL;

    $form[$name]['parents'] = array(
      '#type' => 'hidden',
      '#value' => $pid
    );

    $form[$name]['operations']['edit'] = array(
      '#markup' => l('edit', $entity->uri().'/manage/'.$entity->identifier().'/elements/'.$name.'/edit')
    );

    $form[$name]['operations']['delete'] = array(
      '#markup' => l('edit', $entity->uri().'/manage/'.$entity->identifier().'/elements/'.$name.'/delete')
    );

    // Hurry along the following iteration
    unset($element['title'], $element['name'], $element['type'], $element['weight'], $element['parents']);

    foreach ($element as $n => $item) {
      if (is_array($item)) {
        _fieldform_element_form_row($form, array($n => $item), $entity);
      }
    }
  }
}

function fieldform_elements_list_submit($form, &$form_state) {
  $entity = $form_state['fieldform'];
  $values = $form_state['values'];
  $elements = $values['elements'];
  $add_element = $elements['add_new_element'];
  unset($elements['add_new_element']);

  foreach ($elements as $name => $element) {
    $element['parents'] = explode(':', $element['parents']);
    $old_element = element_get_from_elements($name, $entity->elements);
    $element = array_merge_recursive($old_element, $element);
    $old_element = fieldform_element_create($old_element['type'], $old_element);
    $new_element = fieldform_element_create($element['type'], $element);
    elements_delete($entity, $old_element);
    elements_add($entity, $new_element);
  }

  if (isset($add_element['name'])) {
    $add_element['parents'] = explode(':', $add_element['parents']);
    $new = fieldform_element_create($add_element['type'], $add_element);
    elements_add($entity, $new);
    $form_state['redirect'] = $entity->uri().'/manage/'.$entity->identifier().'/elements/'.$new->name.'/edit';
  }

  entity_save($entity->entityType(), $entity);
  drupal_redirect_form($form_state);
}