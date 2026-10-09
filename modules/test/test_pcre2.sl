() = evalfile ("./test.sl");
require("pcre2.sl");

static define test_regexp_match (slpat, str, eret, eans, opts)
{
   variable pat = slang_to_pcre2 (slpat);
   variable p = pcre2_compile (pat, opts);

   variable ret = pcre2_exec (p, str);
   if (eret != ret)
     failed ("pcre2_exec using '%s' failed: expected %d, got %d",
	     slpat, eret, ret);

   if (eans == NULL)
     return;

   variable ans = pcre2_nth_substr (p, str, 0);
   if (ans != eans)
     failed ("pcre2_exec(%s) matched '%s, expected '%s'",
	     slpat, ans, eans);

   variable matches = pcre2_matches (pat, str);
   if ((matches == NULL) || (matches[0] != eans))
     failed ("pcre2_matches");
}

define slsh_main ()
{
   testing_module ("pcre2");

   test_regexp_match ("B.*[1-5]", "0xAB123G", 1, "B123", 0);
   test_regexp_match ("\([1-5]+\)G\1"R, "0xAB123G12F", 0, NULL, 0);
   test_regexp_match ("\([1-5]+\)G\1"R, "0xAB123G123F", 2, "123G123", 0);
   test_regexp_match ("[1-5]", "0xAB123G", 1, "1", 0);
   test_regexp_match ("\([1-5]2\)"R, "0xAB123G", 2, "12", 0);
   test_regexp_match ("G", "0xAB123G", 1, "G", 0);
   test_regexp_match ("\(G\)"R, "0xAB123G", 2, "G", 0);

   test_regexp_match (`=\(\d\)`, "L=1X", 2, "=1", 0);
   test_regexp_match (`=\(\d?\)`, "L=1X", 2, "=1", 0);
   test_regexp_match (`=\(\d?\)`, "L=12X", 2, "=1", 0);
   test_regexp_match (`=\(\d+\)`, "L=1X", 2, "=1", 0);
   test_regexp_match (`=\(\d+\)`, "L=12X", 2, "=12", 0);
   test_regexp_match (`=\(\d*\)`, "L=1X", 2, "=1", 0);
   test_regexp_match (`=\(\d*\)`, "L=X", 2, "=", 0);

   end_test ();
}
