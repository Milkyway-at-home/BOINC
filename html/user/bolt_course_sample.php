<?php

require_once("../inc/bolt.inc");

function part2() {
    return sequence(
        name('inner seq'),
        lesson(
            name('lesson 3'),
            filename('bolt_sample_lesson.php?n=3')
        )
    );
}

function basic_review() {
    return sequence(
        lesson(
            name('lesson 1'),
            filename('bolt_sample_lesson.php?n=1')
        ),
        lesson(
            name('lesson 2'),
            filename('bolt_sample_lesson.php?n=2')
        )
    );
}

function int_review() {
    return lesson(
        name('lesson 2'),
        filename('bolt_sample_lesson.php?n=2')
    );
}

return sequence(
    name('course'),
    random(
        name('first lessons'),
        number(2),
        lesson(
            name('lesson 1'),
            filename('bolt_sample_lesson.php?n=1')
        ),
        lesson(
            name('lesson 2'),
            filename('bolt_sample_lesson.php?n=2')
        ),
        lesson(
            name('lesson 3'),
            filename('bolt_sample_lesson.php?n=3')
        )
    ),
    exercise_set(
        name('exercise set 1'),
        number(2),
        exercise(
            name('exercise 1'),
            filename('bolt_sample_exercise.php?n=1')
        ),
        exercise(
            name('exercise 2'),
            filename('bolt_sample_exercise.php?n=2')
        ),
        exercise(
            name('exercise 3'),
            filename('bolt_sample_exercise.php?n=3')
        ),
        repeat(.3, basic_review(), REVIEW),
        repeat(.7, int_review(), REVIEW|REPEAT),
        repeat(1, null, REPEAT|NEXT),
        refresh(array(7, 14, 28))
    ),
    part2()
);

?>
