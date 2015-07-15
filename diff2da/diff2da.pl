#!/usr/bin/perl -w
# Neverwinter Nights 2DA diff and reformat utility 
# Version: 1.0
#
# Copyright (C) 2015, Meaglyn <meaglyn.nwn@gmail.com>
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
# Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 
#TODO
#
#    option to suppress header and not labels
#    Fix handling of apostrophes 


use strict;
use Getopt::Std;
use warnings;
#use autodie;
use Text::ParseWords qw( quotewords );

# find where the script is installed.
my $dirname;
BEGIN {
    use File::Basename;
    use Cwd 'abs_path';
    $dirname = dirname(abs_path($0));
} 

use lib "$dirname";
#use lib '/home/pauld/pa/nwntools/perl_2da';
#print "\@INC is @INC\n";


# local libraries here 
use Number::Range;

my $DEBUG = 0;
my $verbose = 0;
my $start_line = 0;
my $quiet = 0;
my $print_header = 1;

my $include_rows; # = Number::Range->new("2,3");
my $exclude_rows; # = Number::Range->new("2,3,100..6000");
my %include_cols = ();

my @include_cols;
my $col_range;
my $have_col_list = 0;

#my $range = Number::Range->new("-10..10,12,100..120");
#  if ($range->inrange("13")) {
#    print "In range\n";
#  } else {
#    print "Not in range\n";
#  }
#  $range->addrange("200..300");
#  $range->delrange("250..255");
#  my $format = $range->range;
  # $format will be '-10..10,12,100..120,200..249,256..300'

sub rowInRange {
    my $row = shift;
    
    if (defined  $include_rows) {
	 $include_rows->inrange("$row");
    } elsif  (defined  $exclude_rows) {
	 !$exclude_rows->inrange("$row");
    } else {
	return 1;
    }
}


sub colInRange {
    my $col = shift;
    
    if (defined $col_range) { 
	return $col_range->inrange($col);
    }
    return 1;
}

sub findColumn {
     my $headref = shift;
     my $label = shift;
     
     print STDERR "DBG findcolum $label in @{$headref}\n"  if ($DEBUG);
     print STDERR "DBG findcolum headers has " .@{$headref}. " items\n" if ($DEBUG);
     # column labels start at 1
     for my $i (1 .. $#{$headref}) { 
	 print STDERR "DBG findcolum $i =  ${$headref}[$i]\n" if ($DEBUG);
	 if (defined  ${$headref}[$i]) {
	     #print STDERR "DBG findcolum $i =  ${$headref}[$i]\n";
	     if (lc($label) eq lc(${$headref}[$i])) {
		 return $i;
	     }
	 }
     }
     return 0;
}

sub setupColumns {
    my $headref = shift;
    my $filename = shift;

    if ($have_col_list) {
	$col_range = Number::Range->new("");
	foreach(@include_cols) {
	    my $col =  findColumn ($headref, $_);
	    if (!$col) {
		print STDERR "File $filename does not contain column \"$_\"\n";
		return 0;
	    }
	    $col_range->addrange("$col");
	}
	print STDERR "Have col range = " . $col_range->range . "\n" if ($DEBUG);
    }
    return 1;
}

# given an array ref return a new ref with only the 
# entries in col_range. 
sub filtercolumns {
    my $arr_ref = shift;
    my $cols = shift;
    if (!$have_col_list) {
	return $arr_ref;
    }

    my @ret;
    for my $i (0 .. $cols) {
	if ($i == 0) {
	    push(@ret, ("${$arr_ref}[$i]"));
	} else {
	    if ( colInRange ($i)) {
		if (!defined ${$arr_ref}[$i]) {
		    push(@ret,("****"));
		} else {
		    push(@ret, ("${$arr_ref}[$i]"));
		}
	    }
	}
    } 
    return \@ret;
}

# checks for space or semi-colon format and opens the file
# returns a hash contianing the file data.
sub open2dafile {
 
    my $lineno = 0;
    my $got_header = 0;
    my $cols = 0;
    my @row;
    my %file2da = ();

    open (my $fh, "<", $_[0]) or die "Unable to open file \"$_[0]\"";
    my @file_array;
    my @header;

    my $type = 0;
    my $count = 0;
    while (<$fh>) {
	chomp;
	my $line = $_;  
	if ($line =~ /\r/) {
	    #print "line has \\r\n";
	    $line =~ s/\r//;
	}

	print STDERR "LINE $line\n" if ($DEBUG);

      	if ($count == 0) {
	    if(!$line eq "2DA V2.0") {
		print STDERR "Error - \"$_[0]\" is not a 2da file\n";
		exit 1;
	    } else {
		$count ++;
		next; 
	    }
	}	
	if (length $line == 0) {
	    next;
	}

	if ($count == 1) {
	    if ($line =~ /;/) {
		print STDERR "Found Semi-colon delimited file\n" if ($DEBUG);
		$type = 1;
	    } else {
		print STDERR "Found Space delimited file\n" if ($DEBUG);
	    }
	    last;
	}
    }

    #reset the file
    #use Fcntl;
    seek $fh, 0, 0;
    while (my $line = <$fh>) {
	chomp $line;    
	if ($line =~ /\r/) {
	    #print "line has \\r\n";
	    $line =~ s/\r//;
	}
	print STDERR "LINE2 \"$line\n\"" if ($DEBUG);

	# remove trailing spaces
	$line =~ s/\s+$//;
	# remove leading spaces once we have the header line
	if ($got_header == 1) {
	    $line =~ s/^\s+//;
	}

	# remove single quotes - they break things
	$line =~ s/\'//;

	# remove any trailing semi-colon 
	if ($type == 1) {
	    if (substr($line, -1) eq ";") {
		chop($line);
	    }
	} 

	my @row;
	# type one is a semi-colon delimited file
	if ($type == 1) {
	    @row = quotewords(';', 0, $line) ;
	} else {
	    @row = quotewords('\s+', 0, $line);
	}

	print STDERR "DBG Got row size " . scalar(@row)." \n" if ($DEBUG);
	print STDERR "line no = $lineno got header = $got_header\n" if ($DEBUG);

	# blank line
	if (!defined $row[0] || (!defined $row[1] && $row[0]=~/^\s*$/)) {
	    #print "DBG blank line - next\n";
	    next; 
        }
	
	#print STDERR "DBG row[0] = \"$row[0]\"\n" if ($DEBUG);
      	if ($lineno == 0 && (($type == 1 && $row[0] eq "2DA V2.0")  || $row[0] eq "2DA")) { 
	    #print "DBG 2DA line - next\n";
	    next; 
	}	

	if ($lineno == 0 && $got_header == 0) { 
	    #print "DBG Header line - next\n";
	    @header = @row;
	    $cols = $#header;
	    $got_header = 1;

	    if ($DEBUG) {
		for my $i (1 .. $cols) {
		    print STDERR "header[$i] = \"$header[$i]\"\n";
		}
	    }

	    # validate headers against list of allowed cols to make sure they are all there
	    if (!setupColumns(\@header, $_[0])) {
		exit 1;
	    }
 
	    if ($DEBUG) {print STDERR "Headers = \"@header\"\n";}
	    next;
	}
	

	for my $i (0 .. $#row) { 
	    next  if (!defined  $row[$i]); 
		if ($row[$i] =~ /\s/ ) {
			$row[$i] = "\"$row[$i]\"";
		}
	}

	# else got a regular row to put it in the array
	push (@file_array, \@row);
	$lineno++;
    }


    $file2da{ 'numcols' } = $cols; 
    $file2da{ 'numrows' } = $lineno;
    $file2da{ 'headers' } = \@header;
    $file2da{ 'rows' } =  \@file_array;
    $file2da{ 'fname' } = $_[0];

    %file2da;
}

# used to space out 2das for printing normal format.
# we find the widest colum entry across all rows and the header
sub calculate_widths {
     my $href = shift;
     my $rowref = $href->{ 'rows' };
     my $numrows = $href->{ 'numrows' };
     my $numcols = $href->{ 'numcols' };
     my $headref = $href->{ 'headers' };
     my @width;
	
     if (!defined  $href->{ 'headers' }) {
	 print STDERR "headers undefined?\n";
     }

     #print "href = $href\n";
     #print "headref = $headref \"@{$headref}\"\n";
     #print STDERR "headers @{$headref} \n";
     $headref =  filtercolumns($headref, $numcols);
     #print STDERR "filtered headers @{$headref} \n";

     # get widths for headers
     for my $i (0 .. $#{$headref}) {
	 next  if (!defined  ${$headref}[$i]);

	 if (${$headref}[$i] =~ /\s/ ) {
			${$headref}[$i] = "\"${$headref}[$i]\"";
		}	
		my $l = length ${$headref}[$i];
		$width[$i] = $l;
		if ($i > 0) {
			if ($width[$i] < 8) { $width[$i] = 8;}
		}
     }

     for my $j (0 .. $numrows -1) {
	 next if (!rowInRange($j)); 
	 my $words = ${$rowref}[$j];
	 $words =  filtercolumns($words, $numcols);
	 for my $i (0 .. $#{$words}) {
	     next if (!defined  ${$words}[$i]);

	     #add quotes if needed
	     if (${$words}[$i] =~ /\s/ && ${$words}[$i] !~ /\"/) {
		 ${$words}[$i] = "\"${$words}[$i]\"";
	     }	
	     my $l = length ${$words}[$i];
	     next if (!defined $width[$i]);
	     #if (!defined  $width[$i]) {  $width[$i] = 0; }
	     $width[$i] = $l if $l > $width[$i];
	     if ($i > 0) {
		 if ($width[$i] < 8) { $width[$i] = 8;}
	     }
	 }
     }
     
     @width;
}


sub print_header {
    print "2DA V2.0\r\n";
    print "\r\n";
}

sub printfile_spaces {
    my $file = shift; 
    my $rowref = $file->{ 'rows' };
    my $numrows = $file->{ 'numrows' };
    my $headref = $file->{ 'headers' }; 
    my $numcols = $file->{ 'numcols' };
    my @width =  calculate_widths($file);

    my $format = join "    ", map { "%-${_}s" } @width;
    $format .= "\r\n";

    #print "numrows = $numrows\n";

    if ($print_header) {
	print_header();
	$headref =  filtercolumns($headref, $numcols);
	printf $format, @{$headref};
    }

    my $needs_pop = 0;
    for my $j (0 .. $numrows - 1) { 
	next if (!rowInRange($j)); 
	
	my $words = ${$rowref}[$j];
	next if (!defined $words);
	$words =  filtercolumns($words, $numcols);
	$needs_pop = 0;
	for my $i (0 .. $#{$words}) {
	    if (!defined  ${$words}[$i]) {
		if ($i == $#{$words}) {
		    $needs_pop = 1;
		}
		next;
	     }
	    if (${$words}[$i] =~ /\s/  && ${$words}[$i] !~ /\"/) {
		${$words}[$i] = "\"${$words}[$i]\"";
	    }
	}

	if ($needs_pop) {
	    my $x = pop  @{$words};
	}

	# suppress error messages here
	{
	    no warnings 'uninitialized';
	    printf $format, @{$words};
	}
    }
}

sub printfile_delim {
    my $file = shift;
    my $delim = shift;
    my $rowref = $file->{ 'rows' };
    my $numrows = $file->{ 'numrows' };
    my $headref = $file->{ 'headers' };
    my $numcols = $file->{ 'numcols' };

     if(defined $delim) {
	 print "got delim \"$delim\"\n"; 
     } else {
	 $delim = ";";
     }

    #print "numrows = $numrows\n";

    if ($print_header) {
	print_header();
	$headref =  filtercolumns($headref, $numcols);
	print join ($delim, @{$headref});
	print "\r\n";
    }

    my $needs_pop = 0;
    for my $j (0 .. $numrows - 1) { 
	next if (!rowInRange($j)); 
	my $words = ${$rowref}[$j];
	next if (!defined $words);
	$words =  filtercolumns($words, $numcols);

	$needs_pop = 0;
	for my $i (0 .. $#{$words}) {
	    if (!defined  ${$words}[$i]) {
		if ($i == $#{$words}) {
		    $needs_pop = 1;
		}
		next;
	     }
	    if (${$words}[$i] =~ /\s/  && ${$words}[$i] !~ /\"/) {
		${$words}[$i] = "\"${$words}[$i]\"";
	    }
	}

	if ($needs_pop) {
	    my $x = pop  @{$words};
	}	
	print join($delim, @{$words});
    	print "\r\n";

    }
}

# check the listed line numbers versus the actual line number.
# report first occurrence of mismatch or all mismatches.
# report lines with extra columns.
sub validate2da {
    my $href = shift; 
    my $cont_on_error  = shift;
    my $fix_errors = shift;

    my $rowref = $href->{ 'rows' };  
    my $numcols = $href->{ 'numcols' };
    my $numrows = $href->{ 'numrows' };
    my $headref = $href->{ 'headers' };
    my $lineno  = 0;
    my $cols = 0;

    if (!defined  $cont_on_error ) {
	$cont_on_error = 0;
    }

    if (!defined $fix_errors) {
	$fix_errors = 0;
    } else {
	# fix errors means continue on error.
	if ($fix_errors) {
	    $cont_on_error = 1;
	}
    }
    if ($start_line != 0) {
	print STDERR "Validating 2DA using $start_line as first row number\n";
    }

    for my $j (0 .. $numrows - 1) { 
	next if (!rowInRange($j)); 
	my $words = ${$rowref}[$j];
	next if (!defined $words);

	my $row = $j + $start_line;
	if (defined ${$words}[0]) {
	    if (${$words}[0] != $row) {
		 print STDERR "Row $j($row) misnumbered: has ${$words}[0]";
		 if ($fix_errors) {
		     ${$words}[0] = $row;
		     print STDERR " - Fixed";
		 } elsif ($cont_on_error == 0) {
		     print STDERR "\n"; 
		     last;
		 }
		 print STDERR "\n"; 
	    } 
	   
	 } else {
	       # this should never happen?
	     print STDERR "Row $j($row): missing row number. 2da file needs renumbering\n";
	     if ($fix_errors) {
		 ${$words}[0] = $row;
		 print STDERR " - Fixed";
	     } elsif ($cont_on_error == 0) { 
		 print STDERR "\n"; 
		 last;
	     }
	     print STDERR "\n"; 
	 }

	$cols = $#{$words};
	if ($cols < $numcols) {
	     print STDERR "Row $j($row) missing column(s). Have $cols, need $numcols";
	     if ($fix_errors) {
		 # insert missing cols at end.
		 while ( $#{$words} < $numcols) {
		     push (@{$words}, ("****"));
		 }

		 print STDERR " - Fixed";
	     } elsif ($cont_on_error == 0) { 
		 print STDERR "\n"; 
		 last;
	     } 
	     print STDERR "\n"; 
	} elsif ($cols > $numcols) { 
	     print STDERR "Row $j($row) extra column(s). Have $cols, need $numcols";
	     if ($verbose) {
		 my $out = " (";
		 for my $l ($numcols..$#{$words}) {
		     $out = $out ." ${$words}[$l]";
		 } 
		 print STDERR $out.")";
	     }
	      if ($fix_errors) {
		 # remove extra cols from end.
		 while ( $#{$words} > $numcols) {
		     pop (@{$words});
		 }

		 print STDERR " - Fixed";
	     } elsif ($cont_on_error == 0) { 
		 print STDERR "\n"; 
		 last;
	     }
	     print STDERR "\n";  
	}	
    }
}

## diff the two files.
sub diff2da {
    my $href = shift; 
    my $rowref = $href->{ 'rows' };  
    my $numcols = $href->{ 'numcols' };
    my $numrows = $href->{ 'numrows' };
    my $headref = $href->{ 'headers' };
    my $fname1  = $href->{ 'fname' };

    my $href2 = shift; 
    my $rowref2 = $href2->{ 'rows' };  
    my $numcols2 = $href2->{ 'numcols' };
    my $numrows2 = $href2->{ 'numrows' };
    my $headref2 = $href2->{ 'headers' };
    my $fname2  = $href2->{ 'fname' };

    $headref =  filtercolumns($headref, $numcols);
    $headref2 =  filtercolumns($headref2, $numcols2);

    # check the number of columns 
    my $cols = $#{$headref};
    my $long_cols =  $#{$headref};
    my $long_cols_ref = $headref;
    my $missing_cols = 0;
    my $missing_cols_fname = $fname1;

    if ($#{$headref} > $#{$headref2}) {
	print "Num Cols  $fname1 :$#{$headref} >  $fname2 :$#{$headref2})\n"; 
	$missing_cols = 1;
	$cols =  $#{$headref2};
	$missing_cols_fname = $fname2;
    } elsif ($#{$headref} < $#{$headref2}) {
	print "Num Cols  $fname1 :$#{$headref} <  $fname2 :$#{$headref2})\n";	
	$missing_cols = 1;
	$long_cols =  $#{$headref2};
	$long_cols_ref = $headref2;
    }
    
    my $out = "";
    # diff the two headref arrays by entry
    for my $i (0 .. $cols) {
	if ( lc(${$headref}[$i]) ne lc(${$headref2}[$i])) {
	    $out = $out . " $i(${$headref}[$i] <> ${$headref2}[$i])";
	}
    }
    # report differences
    if ( $out ne "" ) {
	print "Column labels differ (ignore case) at : $out \n";
    }
    if ($missing_cols) {
	my $out = "$missing_cols_fname missing column(s) : ";
	for my $i (($cols + 1)..$long_cols) {
	    my $col = ${$long_cols_ref}[$i];
	    next if (!defined $col);
	    $out = $out . "\"$col\" ";
	}
	print "$out\n";
    }


    # we will use the smallest number of rows for actual comparisons
    my $rows = $numrows;
    my $long_rows = $numrows;
    my $long_rows_ref = $rowref;
    my $long_fname = $fname1;
    my $missing_rows = 0;
    if ($numrows < $numrows2) {
	print "$fname2 has ". ($numrows2 - $numrows). " more row(s) than $fname1\n";
	$long_rows = $numrows2;
	$long_rows_ref = $rowref2;
	$long_fname = $fname2;
	$missing_rows = 1;
    } elsif  ($numrows > $numrows2) {
	print "$fname1 has ". ($numrows - $numrows2). " more row(s) than $fname2\n";
	$rows =  $numrows2;
	$missing_rows = 1;
    }

    for my $i ( 0..$rows) {
	next if (!rowInRange($i));
	my $row1 = ${$rowref}[$i];
	my $row2 = ${$rowref2}[$i];

	next if (!defined  $row1 || !defined  $row2);
	#print STDERR "got row1 = @{$row1}\n";
	#print STDERR "got row2 = @{$row2}\n";

	my $diff_row = 0;
	my $diff_col = 0;
	my $verbose_col = "";
	for my $j (1..$cols) { 
	    if (lc(${$row1}[$j]) ne lc(${$row2}[$j])) {
		$diff_col ++;
		$verbose_col = $verbose_col. " ${$headref}[$j](${$row1}[$j]|${$row2}[$j])";
	    }	    
	}
	my $out = "Row $i:";
	if ($#{$row1} != $#{$row2}) {
	    $out = $out." num cols ($#{$row1}|$#{$row2}) ";
	    $diff_row ++;
	}
	if ($diff_col) {
	    $diff_row ++;
	    $out = $out. " $diff_col column(s) differ";
	    if ($verbose) {
		$out = $out." $verbose_col";
	    }
	}
	if ($diff_row) {
	    print $out."\n";
	}

    }
    # could print missing rows if doing verbose output
    if ($missing_rows) {
        for my $i ($rows..$long_rows) {
	    next if (!rowInRange($i));
	    my $row1 = ${$long_rows_ref}[$i];
	    next if (!defined $row1);
	    
	    if ($verbose) {
		print "Row $i: Only in $long_fname: \"@{$row1}\"\n";
	    } else {
		print "Row $i: Only in $long_fname\n";
	    }
	}
    } 


}

sub usage {
    print STDERR "Usage : $0 [-afhHpqrSuv] [-n start_line] [-o outfile] [-lL <list>] [ -c <list>] file1 [file2]\n";
    print STDERR "\t a: report all errors when validating\n"; 
    #print STDERR "\t d: enable debug messages (not for the faint of heart)\n"; 
    print STDERR "\t f: try to fix row and column errors, implies -ar\n"; 
    print STDERR "\t h: display this message\n";
    print STDERR "\t H: suppress printout of the header lines (including column labels)\n";
    print STDERR "\t p: print the resulting file (defaults to normal tabbed output)\n";
    print STDERR "\t q: suppress any extra informational messages\n";
    print STDERR "\t r: report file1's row number and column errors\n";
    print STDERR "\t S: print in semicolon separated format (useless without -p) \n";
    print STDERR "\t u: diff file1 and file2. Files should be valid 2da files with no errors.\n";
    print STDERR "\t v: verbose output. Longer error messages and column differences\n";
    print STDERR "\t n <start_line>: use given starting line number for validation. Useful when operating on a subset of a 2da\n";
    print STDERR "\t o <outfile>: Write printed 2da to outfile. Must not be the same as file1 or file2\n";
    print STDERR "\t l <list>: operate on only the listed rows. E.g. -l 1,3,100..110  \n"; 
    print STDERR "\t L <list>: do not operate on the listed rows. E.g. -L 1,3,100..110  \n";  
    print STDERR "\t Note: l and L are mutually exclusive\n";
    print STDERR "\t c <list>: operate on only the listed columns. E.g. -c Label,Value  \n";  
    print STDERR "\t Note: column headers in <list>  case INsensitive (Label == label == LABEL)\n";
    print STDERR "\t Note: columns listed effect diffing and printout but not validation. \n";  
}

########################################
#main code


# declare the perl command line flags/options we want to allow
my %options=();
if (! getopts("ac:C:dfhHl:L:n:o:pqrSuv", \%options)) {
    usage;
    exit 1;
}
 
# test for the existence of the options on the command line.
# in a normal program you'd do more than just print these.
#print "-h $options{h}\n" if defined $options{h};
#print "-j $options{j}\n" if defined $options{j};
#print "-l $options{l}\n" if defined $options{l};
#print "-n $options{n}\n" if defined $options{n};
#print "-s $options{s}\n" if defined $options{s};
 
# other things found on the command line
#print "Other things found on the command line:\n" if $ARGV[0];
#foreach (@ARGV)
#{
#  print "$_\n";
#}

my $op = 0;
my $print = 0; 
my $print_delim = 0;
my $continue = 0;
my $fixerrors = 0;
my $outfile = "";
my $diff = 0;

# print the file
if (defined $options{p}) {
    $print = 1;
}


if (defined $options{S}) {
      # print semi colons
    $print_delim = 1;
}

if (defined $options{d}) {
    $DEBUG = 1;
}
if (defined $options{q}) {
    $quiet = 1;
}

if (defined $options{a}) {
    $continue = 1;
}
if (defined $options{f}) {
    $fixerrors = 1;
}

if (defined $options{H}) {
    $print_header = 0;
}

if (defined $options{v}) {
    $verbose = 1;
}

if (defined $options{n}) {
    $start_line = $options{n};
    # TODO -validate this number
}
if (defined $options{o}) {
    $outfile = $options{o};
}

if (defined $options{l}) {
      $include_rows = Number::Range->new($options{l});

      if (!$quiet) {
	  print STDERR "Including row(s) \"" .$include_rows->range."\"\n";
      }
}

if (defined $options{L}) {
    if (defined $include_rows) {
	print STDERR "Only one of \"-l\" and \"-L\" may be used.\n";
	exit 1;
    }
    $exclude_rows = Number::Range->new($options{l}); 
    if (!$quiet) {
	print STDERR "Excluding row(s) \"" .$exclude_rows->range."\"\n";
    }
}

if (defined $options{c}) {
    @include_cols = quotewords(",",0, $options{c});
    if (scalar(@include_cols) <= 0) {
	print STDERR "No include column specified.\n";
	exit 1;
    } 
    if (!$quiet) {
	print STDERR "Including column(s) \"@include_cols\"\n";
    } 
    $have_col_list = 1;
}





my $cmpfile = "";
if (defined $options{u}) {
    if (!defined $ARGV[1]) {
	print STDERR "No second file for comparison\n";
	usage;
	exit 1;
    }
    $cmpfile = $ARGV[1];
    $diff = 1;
}


if (!defined $ARGV[0]) {
    usage;
    exit 1;
}
my $infile = $ARGV[0];

if (!$outfile eq "") {
    if ($infile eq $outfile) {
	print STDERR "$0 Error : Outfile cannot be same as input file.\n";
	exit 1;
    }
    open (FILE, ">$outfile") or die "Unable to open output file \"$outfile\".\n";
    select FILE;
}

my  %file1 =  open2dafile($infile);


# Diff the two files.
if ($diff) {
    if (!$quiet) {print STDERR "Diffing $infile and $cmpfile\n";}

    my  %file2 =  open2dafile($cmpfile);
    diff2da (\%file1, \%file2);


    exit 0;
}


if (!$quiet) {print STDERR "2da file $infile has $file1{ 'numcols' } columns (excluding row number) and  $file1{ 'numrows' } rows\n";}

if (defined $options{r} || defined $options{f}) {
    validate2da(\%file1, $continue, $fixerrors);
}


# print the file  
if ($print == 1) {
    if ($print_delim == 1) {
	printfile_delim(\%file1);
    } else {
	printfile_spaces(\%file1);
    }
} 
