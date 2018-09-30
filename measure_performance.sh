TIME=$((time emacs --batch -l org \
    --eval "(require 'org)" \
    --eval '(org-babel-tangle-file "worgle.org")') 2>&1 |\
    grep "user" | cut -f 2)

printf "Org-babel-tangle (via Emacs): %s\n" $TIME

TIME=$((time ./worgle -g worgle.org) 2>&1 | grep "user" | cut -f 2)
printf "Worgle: %s\n" $TIME
