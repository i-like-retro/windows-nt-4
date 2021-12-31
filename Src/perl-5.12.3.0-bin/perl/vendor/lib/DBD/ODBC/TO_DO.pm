=head1 NAME

DBD::ODBC::TO_DO - Things to do in DBD::ODBC

As of $LastChangedDate: 2010-10-08 17:00:31 +0100 (Fri, 08 Oct 2010)$

$Revision: 10667 $

=cut

=head1 Todo

  Add array parameter binding (per new DBI Spec)

  Add row caching/multiple row fetches to speed selects

  Better/more tests on multiple statement handles which ensure the
    correct number of rows

  Better/more tests on all queries which ensure the correct number of
    rows and data

  Better tests on SQLExecDirect/do

  Keep checking Oracle's ODBC drivers for Windows to fix the Date
    binding problem

  Add support for $sth->more_results based on DBD::ODBC-specific attribute

  There is a Columns private ODBC method which is not documented.

  Add support for sending lobs in chunks instead of all in one go. Although
    DBD::ODBC uses SQLParamData and SQLPutData internally they are not exposed
    so anyone binding a lob has to have all of it available before it can
    be bound.

  Try to produce a Module::Install build.

  Why does level 15 tracing of any DBD::ODBC script show alot of these:
    !!DBD::ODBC unsupported attribute passed (PrintError)
    !!DBD::ODBC unsupported attribute passed (Username)
    !!DBD::ODBC unsupported attribute passed (dbi_connect_closure)
    !!DBD::ODBC unsupported attribute passed (LongReadLen)

  Add a perlcritic test - see DBD::Pg

  Anywhere we are storing a value in an SV that we didn't create
    (and thus might have magic) should probably set magic.

  Work out how to use Unicode in freeTDS as it does not have SQLW functions
    see examples/freetds_unicode.pl latest CVS trunk apparently has SQLW functions

  Add a test for ChopBlanks and unicode data

  Add a link or documentation on server cursors

  Add some private SQLGetInfo values for whether SQL_ROWSET_SIZE hack
  works etc. How can you tell a driver supports MARS_CONNECTION.

  Might be able to detect MARS capable with SS_COPT_MARS_ENABLED

SQL_COPT_SS_MARS_ENABLED

  Bump requirement to Test::Simple 0.96 so we can use subtest which
  is really cool and reorganise tests to use it. 0.96, because it seems
  to be the first really stable version of subtest.
