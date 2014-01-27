<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 1/16/14
 * Time: 5:26 PM
 */

class FieldFormResultController extends EntityAPIController {
  public function __construct($entity_type) {
    parent::__construct('fieldform_result');
  }

  static public function submit($result = NULL, $form, $form_state) {

  }
}