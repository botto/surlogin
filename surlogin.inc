<?php

/**
 * @file
 * Grouping surlogin core functionality.
 */

/**
 * Creates a user SURLogin key, to be used for logging in with a url link.
 *
 * This will save the mapping between the UID and the SURLogin key.
 *
 * @param int $uid
 *    The userid to generate a key for.
 *
 * @return string
 *    A SURLogin login key, 43 characters long.
 */
function surlogin_create_user_login_key($uid) {
  $login_key = surlogin_generate_login_key();
  // Get the timestamp in future when the login expires.
  // Defaults 90 * 24 * 60 * 60.
  $expire_delta =
    variable_get('surlogin_key_expire_delta', SURLOGIN_EXPIRE_DELTA_DEFAULT);
  // Make sure we are working with numbers.
  if (is_numeric($expire_delta)) {
    $expire = strtotime('today') + $expire_delta;
  }
  else {
    $expire = strtotime('today') + SURLOGIN_EXPIRE_DELTA_DEFAULT;
  }

  $q = db_insert('surlogin')
    ->fields(array(
      'uid' => $uid,
      'login_key' => $login_key,
      'valid_until' => $expire,
    ));
  $q->execute();
  return $login_key;
}

/**
 * Deletes a SURLogin Key by UID.
 *
 * @param mixed $uids
 *    Can either provide an array or a single user id.
 */
function surlogin_delete_by_uid($uids) {
  if (is_numeric($uids)) {
    $uids = array($uids);
  }
  db_delete('surlogin')
    ->condition('uid', $uids, 'IN')
    ->execute();
}

/**
 * Returns a login key object by UID.
 *
 * @param int $uid
 *    The UID to retrive a login key for.
 * @param bool|int $valid_until
 *    Pass the date the key should be valid until.
 *    If no date is passed then the key will be returned even though it's
 *    expired.
 *
 * @return mixed The login key object, otherwise FALSE.
 *    The login key object, otherwise FALSE.
 */
function surlogin_get_login_key_by_uid($uid, $valid_until = FALSE) {
  $q = db_select('surlogin', 's')
    ->fields('s')
    ->condition('uid', $uid, '=');
  if ($valid_until !== FALSE && is_int($valid_until)) {
    $q->condition('valid_until', $valid_until, '>');
  }
  $result = $q->execute()->fetchAllAssoc('uid');
  if (!empty($result) && !empty($result[$uid])) {
    return $result[$uid];
  }
  return FALSE;
}

/**
 * Returns the login key object by key.
 *
 * @param string $key
 *    The login key.
 * @param mixed $valid_until
 *    Pass the date the key should be valid until.
 *    If no date is passed then the key will be returned even though it's
 *    expired.
 *
 * @return mixed The login key object, otherwise FALSE.
 *    The login key object, otherwise FALSE.
 */
function surlogin_get_login_key_by_key($key, $valid_until = FALSE) {
  $q = db_select('surlogin', 's')
    ->fields('s')
    ->condition('login_key', $key, '=');
  if ($valid_until !== FALSE && is_int($valid_until)) {
    $q->condition('valid_until', $valid_until, '>');
  }
  $result = $q->execute()->fetchAllAssoc('login_key');
  if (!empty($result) && !empty($result[$key])) {
    return $result[$key];
  }
  return FALSE;
}

/**
 * Loads the login key and hands over to the login_finalize.
 */
function surlogin_login_key_login_user() {
  global $user;
  $args = arg();
  // If no key is passed then just go straight home.
  if (count($args) < 2) {
    drupal_goto('/');
  }
  $key_string = check_plain($args[1]);

  unset($args[0]);
  unset($args[1]);
  $goto = implode('/', $args);

  // Maintain the original query string.
  $query = $_GET;
  unset($query['q']);
  if (count($query) > 0) {
    $goto .= '?' . implode('&', $query);
  }

  if (strlen($key_string) === 43) {
    $login_key = surlogin_get_login_key_by_key($key_string, time());
    // Only allow users who can have a key generated to also login.
    if (!empty($login_key) && !empty($login_key->uid)) {
      $account_access = user_access('surlogin allow generate key',
        user_load($login_key->uid)
      );
      if ($account_access === TRUE) {
        // If the user is already logged in, then don't bother.
        if (!empty($user) && $user->uid === $login_key->uid) {
          drupal_goto($goto);
        }
        if (!empty($login_key) && !empty($login_key->uid)) {
          $form_state['uid'] = $login_key->uid;
          $form_state['redirect'] = '/';
          // Assign global var to log user in.
          $user = user_load($login_key->uid);
          user_login_finalize($form_state);
          drupal_goto($goto);
        }
      }
    }
  }
  drupal_set_message(
    t('The login url has either expired or does not exist.'), 'error'
  );
  drupal_goto('/');
}