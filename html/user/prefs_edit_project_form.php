<?php

include_once("db.inc");
include_once("util.inc");
include_once("login.inc");
include_once("prefs.inc");

db_init();

$user = get_user_from_cookie();
if ($user == NULL) {
    print_login_form();
} else {
    parse_str(getenv("QUERY_STRING"));
    $prefs = prefs_parse($user->prefs);
    $i = project_index($prefs, $master_url);
    prefs_form_project($prefs->projects[$i], "prefs_edit_project_action.php");
}

?>
