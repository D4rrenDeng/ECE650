## A simple unit test example. Replace by your own tests

import sys
import unittest

class MyTest(unittest.TestCase):

    def test_upper(self):
        """Test the upper() function of class string"""
        self.assertEqual('foo'.upper(), 'FOO')

    def test_isupper(self):
        """Test isupper() function of class string"""
        self.assertTrue('FOO'.isupper())
        self.assertFalse('foo'.isupper())
        self.assertFalse('foo'.isupper())
        self.assertFalse('Foo'.isupper())

    def test_failing(self):
        """A test that fails"""
        self.assertEqual(True, False)

if __name__ == '__main__':
    unittest.main()
