<?php

require_once('../inc/forum.inc');
require_once('../inc/util.inc');

db_init();

$logged_in_user = get_logged_in_user();
$logged_in_user = getForumPreferences($logged_in_user);


if (!get_str('action')) {
    echo "You must specify an action...";
    exit();
}
$thread = getThread(get_int('thread'));
if (!$thread) {
    // TODO: Standard error page
    echo "Invalid thread ID.<br>";
    exit();
}
        
if (!isSpecialUser($logged_in_user,0)) {
    // Can't moderate without being moderator
    echo "You are not authorized to moderate this post.";
    exit();
}


page_head('Forum');
//show_forum_title($forum, $thread, $category->is_helpdesk);

//start form
echo "<form action=forum_moderate_thread_action.php?thread=$thread->id method=POST>\n";
start_table();
row1("Moderate thread");

if (get_str('action')=="hide") {
    //display input that selects reason
    echo "<input type=hidden name=action value=hide>";
    row2("",
    "Select the reason category, optionally write a longer describtion of why you delete the thread and then press ok to delete it.");
    row2("Category",
    "<select name=\"category\">
    <option value=\"1\">Obscene</option>
    <option value=\"2\">Flame/Hate mail</option>
    <option value=\"3\">Commercial spam</option>
    <option value=\"4\">Off Topic</option>
    <option value=\"5\">Flamebait</option>
</select>");
/*} elseif ($_GET['action']=="move") {

    echo "<input type=hidden name=action value=move>";
    row2("Destination thread ID:", "<input name=\"threadid\">");
    //todo display where to move the post as a dropdown instead of having to get ID
*/    
} else {
    echo "Unknown action";
    exit();
}

row2("Reason<br>Mailed if nonempty",
    "<textarea name=\"reason\"></textarea>");

row2(
    "",
    "<input type=\"submit\" name=\"submit\" value=\"OK\">"
);

end_table();

echo "</form>";

page_tail();

?>
