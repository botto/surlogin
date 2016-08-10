<?php

/**
 * @file
 * Pages to interact with SURLogin, i.e. generate and delete keys.
 */

/**
 * User page showing the users SURLogin and gives form to generate SURLogin.
 *
 * @param array $form
 *    Form array.
 * @param array $form_state
 *    Form state array.
 * @param int $uid
 *    User ID.
 *
 * @return string The page form.
 *    The page form.
 */
function surlogin_user_page($form, &$form_state, $uid) {
  module_load_include('inc', 'surlogin');
  $login_key = surlogin_get_login_key_by_uid(intval($uid));
  $account = user_load($uid);
  $form = array();
  $form['login_url'] = array(
    '#type' => 'fieldset',
    '#title' => t('SURLogin'),
  );
  if (!empty($login_key)) {
    if ($login_key->valid_until < time()) {
      drupal_set_message(
        t('The SURLogin has expired, please generate a new SURLogin'),
        'error'
      );
    }

    $form['login_url']['url'] = array(
      '#type' => 'item',
      '#title' => t('You can use the following url to log in:'),
      '#markup' => url('sl/' . $login_key->login_key, array('absolute' => TRUE)),
    );
    $form['login_url']['validity'] = array(
      '#type' => 'item',
      '#title' => t('This key expires on:'),
      '#markup' => format_date($login_key->valid_until, 'short'),
    );
  }
  else {
    if (user_access('surlogin allow generate key', $account) === TRUE
        && $account->uid != 1) {
      $form['login_url']['empty'] = array(
        '#markup' => t('No key found, you can generate one.'),
      );
    }
  }

  if (user_access('surlogin allow generate key', $account) === TRUE
      && $account->uid != 1) {
    $form['generate_key'] = array(
      '#type' => 'submit',
      '#value' => 'Generate Key',
      '#submit' => array('surlogin_user_page_submit_create_login_key'),
      '#validate' => array('surlogin_user_page_validate_create_login_key'),
    );
  }
  else {
    $form['no_permission'] = array(
      '#type' => 'item',
      '#title' => t('This user is not permitted to have a SURLogin.'),
    );
    // Looks like they have one from before.
    if (!empty($login_key)) {
      $form['no_permission']['#markup'] = t('Although a SURLogin is indicated above it will not log the user in and will simply show the standard error for missing SURLogin.');
    }
  }
  $form['delete_key'] = array(
    '#type' => 'submit',
    '#value' => 'Delete Key',
    '#submit' => array('surlogin_user_page_submit_delete_login_key'),
  );
  return $form;
}

/**
 * Checkes if a key is already set.
 *
 * @param array $form
 *    Form array.
 * @param array $form_state
 *    Form state array.
 */
function surlogin_user_page_validate_create_login_key(&$form, &$form_state) {
  module_load_include('inc', 'surlogin');
  $uid = arg(1);
  $login_key = surlogin_get_login_key_by_uid(intval($uid));
  if (!empty($login_key)) {
    form_set_error('login_key', t('Please delete the existing key first.'));
  }
}

/**
 * Submit handler to create a new SURLogin key.
 *
 * @param array $form
 *    Form array.
 * @param array $form_state
 *    Form state array.
 */
function surlogin_user_page_submit_create_login_key(&$form, &$form_state) {
  if (user_access('administer users surlogin') === TRUE) {
    module_load_include('inc', 'surlogin');
    $uid = intval(arg(1));
    if ($uid > 0) {
      surlogin_create_user_login_key($uid);
    }
  }
}

/**
 * Submit handler to delete SURLogin key for the user.
 *
 * @param array $form
 *    Form array.
 * @param array $form_state
 *    Form state array.
 */
function surlogin_user_page_submit_delete_login_key(&$form, &$form_state) {
  if (user_access('administer users surlogin') === TRUE) {
    module_load_include('inc', 'surlogin');
    $uid = arg(1);
    surlogin_delete_by_uid($uid);
  }
}
