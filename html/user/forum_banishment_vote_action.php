<?php
$cvs_version_tracker[]="\$Id: forum_moderate_post_action.php 13718 2007-09-30 11:17:11Z Rytis $";  //Generated automatically - do not edit

/**
 * When a moderator does something to a post, this page actually
 * commits those changes to the database.
 **/

require_once("../inc/forum.inc");
require_once("../inc/forum_email.inc");
require_once("../inc/forum_std.inc");

db_init();

$config = get_config();




$logged_in_user = re_get_logged_in_user();
check_tokens($logged_in_user->getAuthenticator());

if (!$logged_in_user->isSpecialUser(S_MODERATOR)) {
    // Can't moderate without being moderator
    error_page("You are not authorized to banish users.");
}

// See if "action" is provided - either through post or get
if (!post_str('action', true)) {
    if (!get_str('action', true)){
	    error_page("You must specify an action...");
    } else {
	$action = get_str('action');
    }
} else {
    $action = post_str('action');
}

$userid = post_int('userid');
$user=get_user_from_id($userid);

if ($action!="start"){
    error_page("Unknown action ");
}

switch (post_int("category", true)) {
    case 1:
        $mod_category = "Obscene";
    case 2:
        $mod_category = "Flame/Hate mail";
    case 3:
        $mod_category = "User Request";
    default:
        $mod_category = "Other";
}

if (post_str('reason', true)){
        start_vote($config,$logged_in_user,$user, $mod_category,post_str("reason"));
} else { 
        start_vote($config,$logged_in_user,$user, $mod_category,"None given");
}

?>
