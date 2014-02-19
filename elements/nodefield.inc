<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 2/19/14
 * Time: 1:43 PM
 */

class NodeFieldElementController extends FieldFormElementController {
  static public function settingsForm($form, &$form_state, FieldFormElementInterface $element = NULL, $op = NULL) {
    $form = parent::settingsForm($form, $form_state, $element, $op);
    $form['settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('Settings'),
      '#collapsible' => TRUE,
      '#weight' => $form['details']['#weight'] - 0.01,
      '#tree' => TRUE
    );

    $types = node_type_get_types();
    $options = array();
    foreach ($types as $type => $node_type) {
      $options[$type] = $node_type->name;
    }

    $form['settings']['node_type'] = array(
      '#type' => 'select',
      '#title' => t('Node Type'),
      '#options' => $options,
      '#default_value' => isset($form_state['values']['settings']['node_type'])
          ? $form_state['values']['settings']['node_type'] : (
            isset($element->settings['node_type']) ? $element->settings['node_type'] : ''
          ),
      '#multiple' => TRUE
    );

    $form['settings']['multiple'] = array(
      '#type' => 'checkbox',
      '#title' => t('Multiple Values'),
      '#default_value' => isset($form_state['values']['settings']['multiple'])
          ? $form_state['values']['settings']['multiple'] : (
            isset($element->settings['multiple']) ? $element->settings['multiple'] : ''
          ),
      '#states' => array(
        'checked' => array(
          ':input[name="settings[tags]"]' => array(
            'checked' => TRUE
          )
        ),
        'disabled' => array(
          ':input[name="settings[tags]"]' => array(
            'checked' => TRUE
          )
        )
      )
    );

    $form['settings']['tags'] = array(
      '#type' => 'checkbox',
      '#title' => t('Tags'),
      '#description' => t('Implies multiple values but uses a text field, allowing other values than just nodes'),
      '#default_value' => isset($form_state['values']['settings']['tags'])
          ? $form_state['values']['settings']['tags'] : (
            isset($element->settings['tags']) ? $element->settings['tags'] : ''
          )
    );

    return $form;
  }

  static public function buildElement(FieldFormElementInterface $element, FieldFormEntity $entity,
                                      $form = array(), &$form_state = array()) {
    $field = parent::buildElement($element, $entity, $form, $form_state);
    $field['#type'] = 'select';

    if ($element->settings['tags'] == TRUE) {
      $field['#type'] = 'textfield';
      $field['#autocomplete_path'] = 'fieldform/'.$entity->identifier().'/nodefield/'.$element->name;
    }
    elseif ($element->settings['multiple'] == TRUE) {
      $field['#mutliple'] = TRUE;
    }

    if ($field['#type'] == 'select') {
      $field['#options'] = array();
      $nodes = node_load_multiple(array(), array('type' => $element->settings['node_type']));
      foreach ($nodes as $node) {
        $field['#options'][$node->nid] = $node->title;
      }
    }

    return $field;
  }
}

class NodeFieldResultDataController extends FieldFormResultDataController {
  static public function processResult(FieldFormResultDataInterface $result, FieldFormResultEntity $entity,
                                       FieldFormElementInterface $element, $value) {
    if ($element->settings['tags'] == TRUE) {
      $matches = array();
      if (preg_match_all('/\((?<nid>\d+)\)/', $value, $matches) !== FALSE) {
        return $matches['nid'];
      }
    }

    return $value;
  }

  static public function buildElement(FieldFormResultDataInterface $result, &$element, $default_value = NULL) {
    $value = isset($default_value) ? $default_value : $result->value();
    $value = (array)$value;
    if ($element['#type'] == 'textfield') {
      if (!empty($value)) {
        $nodes = node_load_multiple($value);
        $values = array();
        foreach ($nodes as $node) {
          $values[] = t($node->title).' ('.$node->nid.')';
        }
        $element['#default_value'] = implode(',', $values);
      }
    }
    elseif (count($value) == 1 || (!isset($element['#multiple']) || $element['#mutliple'] != TRUE)
      && in_array($value, array_keys($element['#options'])) !== FALSE) {
      $element['#default_value'] = array_shift($value);
    }
    elseif (in_array($value, array_keys($element['#options'])) !== FALSE) {
      $element['#default_value'] = $value;
    }
  }

  static public function preRender(FieldFormResultDataInterface $result, &$vars) {
    $value = (array)$result->value();
    if (!empty($value)) {
      $nodes = node_load_multiple($value);
      $titles = array();
      foreach ($nodes as $node) {
        $titles[] = t($node->title);
      }
      $vars['content'] = implode(', ', $titles);
    }
  }
}

function fieldform_nodefield_autocomplete($entity, $name, $string = '') {
  $element = $entity->findElementByName($name);
  $type = $element->settings['node_type'];
  $matches = array();
  $results = db_select('node', 'n')
    ->fields('n', array('nid', 'title'))
    ->condition('type', $type)
    ->condition('title', db_like($string).'%', 'LIKE')
    ->range(0, 10)
    ->execute()->fetchAllAssoc('nid');

  foreach ($results as $nid => $result) {
    $matches[$nid] = t($result['title']).' ('.$nid.')';
  }

  drupal_json_output($matches);
}