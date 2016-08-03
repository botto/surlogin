<?php

/**
 * @file
 * Admin pages file. Group all admin pages in one file.
 */

/**
 * Page showing config for SURLogin.
 *
 * @param array $form
 *    The form array.
 * @param array $form_state
 *    The form state array.
 *
 * @return array
 *    The form decleration.
 */
function surlogin_admin_page_form($form, &$form_state) {
  // form_load_include($form_state, 'inc', 'surlogin', 'surlogin.admin');.
  $expire_delta_days = variable_get('surlogin_key_expire_delta', SURLOGIN_EXPIRE_DELTA_DEFAULT) / (24 * 3600);
  $form = array();
  $form['key_expire_delta_days'] = array(
    '#title' => t('How many days is each SURLogin valid for'),
    '#type' => 'textfield',
    '#default_value' => $expire_delta_days,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#validate' => array('surlogin_admin_page_form_validate'),
    '#submit' => array('surlogin_admin_pager_form_submit'),
  );
  return $form;
}

/**
 * Validate the days are numbers and whole.
 *
 * @param array $form
 *    The form array.
 * @param array $form_state
 *    The form state array.
 */
function surlogin_admin_page_form_validate($form, &$form_state) {
  if (is_int($form_state['values']['key_expire_delta_days']) === FALSE) {
    form_set_error('key_expire_delta_days',
      t('The number of days has to be a whole number')
    );
  }
  if ($form_state['values']['key_expire_delta_days'] < 1) {
    form_set_error('key_expire_delta_days',
      t('The login has to be valid for at least 1 day.')
    );
  }
}

/**
 * Save the whole days.
 *
 * @param array $form
 *    The form array.
 * @param array $form_state
 *    The form state array.
 */
function surlogin_admin_pager_form_submit($form, &$form_state) {
  $days = $form_state['values']['key_expire_delta_days'];
  $seconds = $days * 24 * 3600;
  variable_set('surlogin_key_expire_delta', $seconds);
  drupal_set_message(t('Updated SURLogin valid link days to @days',
    array('@days' => $days)
    ),
    'info'
  );
}
