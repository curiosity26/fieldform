<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 2/4/14
 * Time: 3:41 PM
 */

class EmailElementController extends FieldFormElementController {
  public function create($values = array()) {
    $element = parent::create($values);
    if (!isset($element->settings) || !is_array($element->settings)) {
      $element->settings = array(
        'anonymous_account' => array(
          'user_lookup' => FALSE,
          'user_create' => FALSE
        ),
        'authenticated_account' => array(
          'hide_field' => FALSE
        )
      );
    }
    return $element;
  }

  static public function buildElement(FieldFormElementInterface $element) {
    $user = $GLOBALS['user'];
    $form = parent::buildElement($element);
    $form['#type'] = 'textfield';
    unset($form['#settings']);
    if ($user->uid > 0 && $element->settings['authenticated_account']['hide_field'] == TRUE) {
      $form['#type'] = 'hidden';
      $form['#value'] = $user->mail;
      unset($form['#default_value']);
    }
    return $form;
  }

  static public function validate(FieldFormElementInterface $element, &$form, &$form_state) {
    $form_element = &drupal_array_get_nested_value($form, $element->parents);
    if (isset($form_element['#parents']) && is_array($form_element['#parents'])) {
      $value = drupal_array_get_nested_value($form_state['values'], $form_element['#parents']);
      if ($form_element['#required'] == TRUE && filter_var($value, FILTER_VALIDATE_EMAIL) === FALSE) {
        form_error($form_element, t('Invalid email address'));
      }
    }
  }

  static public function settingsForm($form, &$form_state, FieldFormElementInterface $element = NULL, $op = NULL) {
    $form = parent::settingsForm($form, $form_state, $element, $op);

    $form['required']['#states']['checked'][':input[name="settings[anonymous_account][user_lookup]"]'] = array(
      'checked' => TRUE
    );

    $form['settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('Settings'),
      '#collapsible' => TRUE,
      '#weight' => $form['details']['#weight'] - 0.01,
      '#tree' => TRUE
    );

    $form['settings']['anonymous_account'] = array(
      '#type' => 'fieldset',
      '#title' => t('Anonymous Users'),
      '#collapsible' => TRUE,
      '#tree' => TRUE
    );

    $form['settings']['anonymous_account']['user_lookup'] = array(
      '#type' => 'checkbox',
      '#title' => t('Lookup site user using this email address'),
      '#decription' => t('If a user is found, the results will be associated with the user'),
      '#default_value' => $element->settings['anonymous_account']['user_lookup']
    );

    $form['settings']['anonymous_account']['user_create'] = array(
      '#type' => 'checkbox',
      '#title' => t('If no user is found, create a new user'),
      '#description' => t('After this field is submitted, a new user will be created using the email address as the
        username and the user will be automatically logged if permitted in the !account_settings',
          array('!account_settings' => l('account settings', 'admin/config/people/accounts'))),
      '#default_value' => $element->settings['anonymous_account']['user_create'],
      '#states' => array(
        'visible' => array(
          ':input[name="settings[anonymous_account][user_lookup]"]' => array('checked' => TRUE)
        ),
        'unchecked' => array(
          ':input[name="settings[anonymous_account][user_lookup]"]' => array('unchecked' => TRUE)
        )
      )
    );

    $form['settings']['authenticated_account'] = array(
      '#type' => 'fieldset',
      '#title' => t('Authenticated Users'),
      '#collapsible' => TRUE,
      '#tree' => TRUE
    );

    $form['settings']['authenticated_account']['hide_field'] = array(
      '#type' => 'checkbox',
      '#title' => t('Hide this field and use the authenticated user\'s email address'),
      '#default_value' => $element->settings['authenticated_account']['hide_field'],
    );

    return $form;
  }
}

class EmailResultDataController extends FieldFormResultDataController {

  static public function buildElement(FieldFormResultDataInterface $result, &$element, $default_value = NULL) {
    $user = $GLOBALS['user'];
    $default_value = isset($default_value) ? $default_value : $result->value();

    if ($element['#type'] === 'hidden') {
      if ($default_value !== $user->mail) {
        $element['#type'] = 'textfield';
        $element['#default_value'] = $default_value;
        unset($element['#value']);
      }
      else {
        $element['#value'] = $default_value;
      }
    }
    else {
      $element['#default_value'] = $default_value;
    }
}

  static public function processResult(FieldFormResultDataInterface $result, FieldFormResultEntity $entity,
                                       FieldFormElementInterface $element, $value) {
    $user = $GLOBALS['user'];
    if ($user->uid == 0 && $element->settings['anonymous_account']['user_lookup'] == TRUE) {
      if (($account = user_load_by_mail($value)) !== FALSE) {
        $state['values']['name'] = $account->name;
        user_login_name_validate(array(), $state); // Check to ensure the user isn't blocked
        if (!form_get_errors()) {
          $entity->uid = $account->uid;
        }
      }
      elseif($element->settings['anonymous_account']['user_create'] == TRUE) {
        $password = user_password();
        $fields = array(
          'name' => $value,
          'mail' => $value,
          'pass' => $password,
          'status' => 1,
          'init' => 'email address',
          'timezone' => drupal_get_user_timezone(),
          'roles' => array(
            DRUPAL_AUTHENTICATED_RID => 'authenticated user',
          )
        );
        $account = user_save('', $fields);
        if ($account !== FALSE) {
          $state['values']['name'] = $account->name;
          user_login_name_validate(array(), $state); // Check to ensure the user isn't blocked
          if (!form_get_errors()) {
            $auth = user_authenticate($account->name, $password);
            drupal_mail('user', 'register_no_approval_required', $value, NULL, array('account' => $account));
            $entity->uid = $auth;
            if ($auth !== FALSE && !variable_get('user_email_verification', TRUE)) {
              $GLOBALS['user'] = $account;
              user_login_finalize();
            }
          }
        }
      }
    }

    return parent::processResult($result, $entity, $element, $value);
  }
}