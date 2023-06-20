from setuptools import setup

setup(
	name='ane',
	version='1.0.6',
	description='ANE driver interface',
	author='Eileen Yoon',
	author_email='eyn@gmx.com',
	packages=['ane'],
	install_requires=['construct', 'numpy'],
)
