<?php
// Where the moderator decides what action to take on a thread.  Sends
// its data to forum_moderate_thread_action.php for the actual action to
// take place.

require_once('../inc/forum.inc');

$logged_in_user = get_logged_in_user();
BoincForumPrefs::lookup($logged_in_user);

if (!get_str('action')) {
    error_page("You must specify an action...");
}
$thread = BoincThread::lookup_id(get_int('thread'));
        
if (!$logged_in_user->prefs->privilege(S_MODERATOR)) {
    error("You are not authorized to moderate this post.");
}

page_head('Forum');

echo "<form action=forum_moderate_thread_action.php?thread=$thread->id method=POST>\n";
echo form_tokens($logged_in_user->authenticator);
start_table();
row1("Moderate thread");

if (get_str('action')=="hide") {
    //display input that selects reason
    echo "<input type=hidden name=action value=hide>";
    row2("",
    "Select the reason category, optionally write a longer description of why you delete the thread and then press ok to delete it.");
    row2("Category",
    "<select name=\"category\">
    <option value=\"1\">Obscene</option>
    <option value=\"2\">Flame/Hate mail</option>
    <option value=\"3\">Commercial spam</option>
    <option value=\"4\">Other</option>
</select>");
} elseif (get_str('action')=="move") {

    echo "<input type=hidden name=action value=move>";
    
    $selectbox = '<select name="forumid">';  
    $categories = BoincCategory::enum();
    foreach ($categories as $category) {
        $forums = BoincForum::enum("category=$category->id");
        foreach ($forums as $forum) {
            $selectbox .= '<option value="'.$forum->id.'">'.$forum->title.'</option>';  
        }  
    }  
    $selectbox .= '</option>';  
    
    row2("Destination forum:", $selectbox);  
} elseif (get_str('action')=="title") {

    echo "<input type=hidden name=action value=title>";
    row2("New title:", "<input name=\"newtitle\" value=\"".stripslashes(htmlspecialchars($thread->title))."\">");
} else {
    error_page("Unknown action");
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
