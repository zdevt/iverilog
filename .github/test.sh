#!/usr/bin/env sh

git clone https://github.com/steveicarus/ivtest.git || exit 1
cd ivtest

version=devel

status=0
if [ "$OS" = 'Windows_NT' ]; then
  perl update_msys2_report.pl $version
  perl vvp_reg.pl || status=1
  diff --strip-trailing-cr regression_report-msys2-$version.txt regression_report.txt || status=1
  echo cat regression_report-msys2-$version.txt
  cat regression_report-msys2-$version.txt
  echo cat gold/pr3366217g.gold
  cat gold/pr3366217g.gold
  echo cat log/pr3366217g.log
  cat log/pr3366217g.log
  echo diff gold/pr3366217g.gold log/pr3366217g.log
  diff gold/pr3366217g.gold log/pr3366217g.log
  echo diff --strip-trailing-cr gold/pr3366217g.gold log/pr3366217g.log
  diff --strip-trailing-cr gold/pr3366217g.gold log/pr3366217g.log
else
  perl vvp_reg.pl || status=1
  diff regression_report-$version.txt regression_report.txt || status=1
fi

perl vpi_reg.pl || status=1

exit $status
