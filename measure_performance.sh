TIME=$((time emacs --batch -l org \
    --eval "(require 'org)" \
    --eval '(org-babel-tangle-file "worgle.org")') 2>&1 |\
    grep "user" | cut -f 2)

printf "Org-babel-tangle (via Emacs): %s\n" $TIME

TIME=$((time ./worgle -g worgle.org) 2>&1 | grep "user" | cut -f 2)
printf "Worgle: %s\n" $TIME

TIME=$((time emacs --batch -f package-initialize -l org \
    --eval '(find-file "worgle.org")' \
    --eval '(org-html-export-to-html)') 2>&1 |\
    grep "user" | cut -f 2)

printf "Org-export-to-html (via Emacs): %s\n" $TIME

TIME=$((time ./sorg < worgle.org > worgle.html) 2>&1 | grep "user" | cut -f 2)
printf "Sorg: %s\n" $TIME
