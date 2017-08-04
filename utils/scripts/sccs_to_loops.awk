{
  if(NR == 1)
    next;
  
  s2l[$2] = s2l[$2]" "$1;
}
END {
  for(s in s2l) {
    printf("%d %s\n", s, s2l[s]);
  }
}
