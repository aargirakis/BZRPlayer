#!/usr/bin/perl

use strict;
use HTML::Template;

# $0 template_name function_file

#main
my $templateName = shift or die 'No template name';
my $listName = shift or die 'No functions filename';
my $outputName = shift or die 'No output filename';
open FUNCS, '<'.$listName or die 'No functions file';
open OUTPUT, '>'.$outputName or die 'Failed to open output file';

my $template = HTML::Template->new(filename => $templateName, die_on_bad_params => 0, global_vars => 1) or die 'Failed to open template';
my @functions = ();

while (my $line = <FUNCS>)
{
  $line =~ /^(.*?)(#.*)?$/;
  my $func = $1;
  next unless $func;
  if ($func =~ /(\w+)\s*=\s*(.+)/)
  {
    $template->param($1 => $2);
    next;
  }
  die "Invalid function format ($_)" unless $func =~ /([\w_][\w\d_*\s]*?)([\w_][\w\d_]*)\s*\((.*)\)/;
  my %function = (DECLARATION => $func, RETTYPE => $1, NAME => $2);
  my @params = split /,\s*/,$3;
  my @paramTypes = ();
  my @paramNames = ();
  while (my $param = shift @params)
  {
    next if $param =~ /^\s*void\s*$/;
    die "Invalid parameters format ($param)" unless $param =~ /^([\w\d_\s*]+?)\s*([\w_][\w\d_]*)(\[\])?$/;
    push @paramTypes, $1.$3;
    push @paramNames, $2;
  }
  $function{PARAMTYPES} = join (', ', @paramTypes);
  $function{PARAMNAMES} = join (', ', @paramNames);
  push(@functions, \%function);
}

$template->param(FUNCTIONS => \@functions);
print OUTPUT $template->output();
