<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
/**
 * This is the forum index
 * It shows the categories available and each of the forums that are
 * contained in those categories
 **/

require_once('../inc/forum.inc');
require_once('../inc/forum_std.inc');
db_init();

$logged_in_user = get_logged_in_user(false);

// Process request to mark all posts as read
if ((get_int("read", true) == 1) && ($logged_in_user)) {
    check_tokens($logged_in_user->authenticator);
    mysql_query("UPDATE forum_preferences SET mark_as_read_timestamp=".time()." WHERE userid=".$logged_in_user->id.";");
    Header("Location: ".get_str("return", true));
}

function forum_summary($forum) {
    echo "
        <tr class=\"row1\">
        <td>
            <em>
            <a href=\"forum_forum.php?id=".$forum->getID()."\">".$forum->getTitle()."
            </a></em>
            <br><span class=\"smalltext\">".$forum->getDescription()."</span>
        </td>
        <td>".$forum->getThreadCount()."</td>
        <td>".$forum->getPostCount()."</td>
        <td>".time_diff_str($forum->getLastTimestamp(), time())."</td>
    </tr>";
}

page_head(tra("%1 Message boards", PROJECT));

echo "
    <p>
    If you have a question or problem, please use the
    <a href=forum_help_desk.php>Questions & answers</a>
    area instead of the Message boards.
    </p>
";

show_forum_title(NULL, NULL);
start_forum_table(array(tra("Topic"), tra("Threads"), tra("Posts"), tra("Last post")));

$categories = $mainFactory->getCategories();
$i=0;
while (isset($categories[$i])) {
    if (!$categories[$i]->getType()) {
        echo '
            <tr class="subtitle">
                <td class="category" colspan="4">'.$categories[$i]->getName().'</td>
            </tr>
        ';
        $forums = $categories[$i]->getForums();
        $ii=0;
        // Show a summary of each of the forums
        while (isset($forums[$ii])) {
            echo forum_summary($forums[$ii]);
            $ii++;
        }
    }
    $i++;
}

end_table();
page_tail();
flush();
cleanup_forum_log();
?>
