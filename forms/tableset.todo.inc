<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 2/6/14
 * Time: 2:44 PM
 */

function fieldform_element_info() {
  return array(
    'tableset' => array(
      '#input' => TRUE,
      '#tree' => TRUE,
      '#process' => array('form_tableset_process'),
      '#header' => array(),
      '#rows' => array(),
      '#attributes' => array(),
      '#caption' => NULL,
      '#title_display' => 'before',
      '#theme' => 'tableset'
    )
  );
}

function form_tableset_process($element, &$form_state, $complete_form) {
  foreach ($element['#rows'] as $key => &$row) {
    if (is_array($row)) {
      foreach ($row as &$col) {
        $default = array('#parents' => array_merge($element['#parents'], array($key)));
        if (!is_array($col)) {
          $name = strtolower(preg_replace('/[^a-zA-Z0-9]+/', '_', $col));
          $default['#parents'][] = $name;
          $col['#data']  = array($name => array('#value' => $col)) + $default;
          drupal_process_form($complete_form['form_id']['#value'], $col['#data'][$name], $form_state);
        }
        else {
          if (!empty($col['data'])) {
            $col['#data'] = $col['data'];
            unset($col['data']);
            if (is_array($col['#data'])) {
              $name = array_keys($col['#data'])[0];
              $default['#parents'][] = $name;
              $col['#data'][$name] += $default;
              drupal_process_form($complete_form['form_id']['#value'], $col['#data'][$name], $form_state);
            }
            else {
              $name = strtolower(preg_replace('/[^a-zA-Z0-9]+/', '_', (string)$col['#data']));
              $default['#parents'][] = $name;
              $col['#data'] = array($name => array('#markup' => $col['#data'])) +  $default;
              drupal_process_form($complete_form['form_id']['#value'], $col['#data'][$name], $form_state);
            }
          }
          else {
            $col['#data'] = array();
            foreach(element_children($col) as $child) {
              $default['#parents'][] = $child;
              if (is_array($col[$child])) {
                $col['#data'][$child] = $col[$child] + $default;
                unset($col[$child]);
                drupal_process_form($complete_form['form_id']['#value'], $col['#data'][$child], $form_state);
              }
            }
          }
        }
      }
    }
  }
  return $element;
}

function fieldform_theme() {
  $themes = array();

  $themes['tableset'] = array(
    'render element' => 'element'
  );
}

function theme_tableset($variables) {
  $element = $variables['element'];
  foreach($element['#rows'] as &$row) {
    if (isset($row['data'])) {
      $data = &$row['data'];
    }
    else {
      $data = &$row;
    }
    foreach($data as &$col) {
      $col['data'] = drupal_render($col['#data']);
      unset($col['#data']);
      if (!empty($col['#attributes'])) {
        $col += $col['#attributes'];
        unset($col['#attributes']);
      }
    }
  }
  $output = theme('table', array(
      'header' => $element['#header'],
      'rows' => $element['#rows'],
      'attributes' => $element['#attributes'],
      'caption' => $element['#caption']
    )
  );
  return theme('form_element', $element, $output);
}