<?php

require_once("../inc/util.inc");
require_once("../inc/uotd.inc");
require_once("../inc/profile.inc");

db_init();

$profile = get_current_uotd();
if (!$profile) {
    echo "No user of the day has been chosen.";
} else {
    $d = date("d F Y", time());
    $user = lookup_user_id($profile->userid);
    page_head("User of the Day for $d: $user->name");
    start_table();
    show_profile($user, get_logged_in_user(false));
    end_table();
}

page_tail();
?>
