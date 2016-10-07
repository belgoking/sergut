import re
import sys

class StdStringPrinter(object):

	def __init__(self, val):
		self.val = val

	def to_string(self):
		ptr = self.val['beginPtr']
		length = self.val['endPtr'] - ptr
		return ptr.lazy_string(length=length)

	def display_hint(self):
		return 'string'

def str_lookup_function(val):
	lookup_tag = val.type.tag
	if lookup_tag == None:
		return None
	regex = re.compile("^sergut::misc::ConstStringRef$")
	if regex.match(lookup_tag):
		return StdStringPrinter(val)
	regex = re.compile("^sergut::misc::StringRef$")
	if regex.match(lookup_tag):
		return StdStringPrinter(val)
	return None

def register_printers(objfile):
	objfile.pretty_printers.append(str_lookup_function)

register_printers(gdb.current_objfile())

