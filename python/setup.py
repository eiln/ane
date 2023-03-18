from setuptools import setup

setup(
   name='ane',
   version='1.0.3',
   description='ANE driver interface',
   author='Eileen Yoon',
   author_email='eyn@gmx.com',
   packages=['ane'],  # same as name
   install_requires=['numpy'], # external packages as dependencies
)
