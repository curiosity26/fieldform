<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 1/16/14
 * Time: 5:26 PM
 */

class FieldFormResultEntity extends Entity {
  public $data = array();
  public function __construct($values = array()) {
    parent::__construct($values, 'fieldform_result');
  }
}