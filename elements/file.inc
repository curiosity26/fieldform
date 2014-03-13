<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 2/7/14
 * Time: 8:46 AM
 */

class FileElementController extends FieldFormElementController {
  static public function settingsForm($form, &$form_state, FieldFormElementInterface $element = NULL, $op = NULL) {
    $form = parent::settingsForm($form, $form_state, $element, $op);
    $form['#attributes']['enctype'] = 'multipart/form-data';
    $form['default_value']['#prefix'] = '<div id="default-value-element-ajax">';
    $form['default_value']['#suffix'] = '</div>';

    $form['settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('Settings'),
      '#collapsible' => TRUE,
      '#tree' => TRUE,
      '#weight' => $form['details']['#weight'] - 0.01
    );

    $form['settings']['is_image'] = array(
      '#type' => 'checkbox',
      '#title' => t('Image'),
      '#default_value' => isset($element->settings['is_image']) ? $element->settings['is_image'] : NULL,
    );

    $form['settings']['extensions'] = array(
      '#type' => 'textfield',
      '#title' => t('Valid File Extensions'),
      '#default_value' => isset($element->settings['extensions'])
          ? $element->settings['extensions'] : 'jpg jpeg gif png txt doc xls pdf ppt pps odt ods odp',
    );

    $form['settings']['path'] = array(
      '#type' => 'textfield',
      '#title' => t('Folder Path'),
      '#description' => t('Files uploaded using this element will be saved to the above folder path within %root',
        array('%root' => file_build_uri(''))),
      '#element_prefix' => file_default_scheme().'://'
    );

    return $form;
  }

  public static function settingsSubmitForm(&$form, &$form_state, $entity_type, $entity, $element_type, $element) {
    if ($form_state['values']['settings']['is_image'] == TRUE) {
      $matches = array();
      if (preg_match_all('/(?<valid>jpe?g|gif|png)/',
          $form_state['values']['settings']['extensions'], $matches) !== FALSE) {
        $form_state['values']['settings']['extensions'] = implode(' ', $matches['valid']);
      }
    }
    parent::settingsSubmitForm($form, $form_state, $entity_type, $entity, $element_type, $element);
  }

  static public function buildElement(FieldFormElementInterface $element, FieldFormEntity $entity,
                                      $form = array(), &$form_state = array()) {
    $output = parent::buildElement($element, $entity, $form, $form_state);
    // Have to format the name so file_save_upload can see it. This is just retarded
    $output['#name'] = 'files['.$output['#name'].']';
    $output['#type'] = 'managed_file';
    $output['#upload_validators'] = $output['#settings']['extensions'];
    $output['#upload_location'] = file_build_uri($output['#settings']['path']);
    unset($output['#settings']);
    return $output;
  }
}

class FileResultDataController extends FieldFormResultDataController {

  static public function buildElement(FieldFormResultDataInterface $result, &$element, $default_value = NULL) {
    $value = $result->value();
    $value = !!$value ? $value : $default_value;
    $element['#default_value'] = $value;
  }

  static public function processResult(FieldFormResultDataInterface $result, FieldFormResultEntity $entity,
                                       FieldFormElementInterface $element, $file) {
    return isset($file['fid']) ? $file['fid'] : $result->value();
  }

  static public function preRender(FieldFormResultDataInterface $result, &$vars) {
    $file = file_load($result->value());
    $url = file_create_url($file->uri);
    $vars['classes_array'][] = 'no-print';
    if (!!$file) {
        $vars['content'] = theme('file_icon', array('file' => $file)).l($file->filename, $url);
    }
  }
}