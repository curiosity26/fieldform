<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 2/10/14
 * Time: 5:31 PM
 */

class RadiosResultDataController extends FieldFormResultDataController {
  static public function processResult(FieldFormResultDataInterface $result, FieldFormResultEntity $entity,
                                       FieldFormElementInterface $element, $value) {
    return isset($element->options[$value]) ? $element->options[$value] : NULL;
  }

  static public function buildElement(FieldFormResultDataInterface $result, &$element, $default_value = NULL) {
    if (!isset($default_value) && isset($element['#options'])) {
      $value = $result->value();
      if (($key = array_search($value, $element['#options'])) !== FALSE) {
        $element['#default_value'] = $key;
      }
    }
  }
}