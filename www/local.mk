SUBDIRS		:= `find .. -maxdepth 1 -type d | grep '\.\./' | grep -v '^\.\./index$$'`
