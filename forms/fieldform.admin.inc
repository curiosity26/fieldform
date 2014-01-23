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
  $form_state['redirect'] = 'admin/structure/fieldform';
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
  $values = isset($form_state['values']) ? $form_state['values'] : array();
  $elements = $entity->elements;

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
    '#default_value' => isset($values['elements']['add_new_element']) ?
        $values['elements']['add_new_element']['type'] : NULL
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
  $base_url = $entity->entityInfo()['admin ui']['path'];
  $temp = array();
  foreach ($elements as $name => $element) {
    if (!$name) {
      continue;
    }
    $pid = !empty($element['parents']) ? implode('][', $element['parents']) : $name;
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
      '#value' => $element['type']
    );

    $temp['weight'] = array(
      '#type' => 'weight',
      '#value' => is_numeric($element['weight']) ? $element['weight'] : 0
    );

    $temp['parents'] = array(
      '#type' => 'hidden',
      '#value' => $pid
    );

    $temp['operations']['edit'] = array(
      '#markup' => l('edit', $base_url.'/manage/'.$entity->identifier().'/elements/'.$name.'/edit')
    );

    $temp['operations']['delete'] = array(
      '#markup' => l('delete', $base_url.'/manage/'.$entity->identifier().'/elements/'.$name.'/delete')
    );

    $form['elements'][$name] = $temp;

    // Hurry along the following iteration
    unset($element['title'], $element['name'], $element['type'],
      $element['weight'], $element['parents'], $element['operations']);

    foreach ($element as $n => $item) {
      if (is_array($item) && isset($item['name']) && isset($item['type'])) {
        _fieldform_element_form_row($form, array($n => $item), $entity);
      }
    }
  }
}

function fieldform_elements_name_exists($value, $element, $form_state) {
  $entity = $form_state['fieldform'];
  return !(empty($entity->elements) || count(element_get_from_elements($value, $entity->elements)) > 0);
}

function fieldform_elements_list_submit($form, &$form_state) {
  $entity = $form_state['fieldform'];
  $form_state['redirect'] = $entity->entityInfo()['admin ui']['path'].'/manage/'.$entity->identifier().'/elements';
  $values = $form_state['values'];
  $elements = $values['elements'];
  uasort($elements, 'drupal_sort_weight');

  foreach ($elements as $name => $element) {
    $new_element = fieldform_element_create($element['type'], $element);
    if ($name === 'add_new_element' && $new_element !== FALSE) {
      $form_state['redirect'] = $entity->entityInfo()['admin ui']['path'].'/manage/'.$entity->identifier().
        '/elements/'.$new_element->name.'/edit';
    }
    elseif ($name) {
      $old_element = element_get_from_elements($name, $entity->elements);
      if (!empty($old_element) && isset($old_element['name']) && isset($old_element['type'])) {
        if (($old_element = fieldform_element_create($old_element['type'], $old_element)) !== FALSE) {
          elements_delete($entity, $old_element);
        }
      }
    }
    if ($new_element !== FALSE) {
      elements_add($entity, $new_element);
      entity_save($entity->entityType(), $entity); // Prevent unnecessary hits to the database by putting this here
    }
  }

  drupal_redirect_form($form_state);
}