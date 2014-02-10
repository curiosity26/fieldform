<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 2/10/14
 * Time: 5:31 PM
 */

class SelectResultDataController extends OptionsResultDataController {
  static public function buildElement(FieldFormResultDataInterface $result, &$element, $default_value = NULL) {
    if (!isset($default_value) && isset($element['#options'])) {
      $value = $result->value();
      if (is_array($value)) {
        $ret = array();
        foreach ($value as $v) {
          if (($key = array_search($v, $element['#options'])) !== FALSE) {
            $ret[] = $key;
          }
        }
        if (!empty($key)) {
          $element['#default_value'] = $ret;
        }
      }
      elseif (($key = array_search($value, $element['#options'])) !== FALSE) {
        $element['#default_value'] = $key;
      }
    }
  }
}