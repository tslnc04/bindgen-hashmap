//! Rust wrapper for a basic implementation of a hashmap in C

#![warn(
    missing_docs,
    clippy::missing_docs_in_private_items,
    rustdoc::broken_intra_doc_links
)]
use std::{
    ffi::{c_void, CString},
    marker::PhantomData,
};

mod hashmap;

/// Basic hashmap implementation wrapping the C hashmap. Buckets are doubled
/// whenever the load factor exceeds 0.75.
pub struct HashMap<V> {
    /// Pointer to the C hashmap we're wrapping
    ptr: *mut hashmap::HashMap,
    /// Phantom data to show we're using V even if it's not clear from types
    /// alone because of ffi
    value: PhantomData<V>,
}

impl<V> HashMap<V> {
    /// Create a new hashmap with 0 buckets. It will be initialized to 8 buckets
    /// on first insertion.
    pub fn new() -> Self {
        Self::default()
    }

    /// Create a new hashmap with the specified number of buckets.
    pub fn with_buckets(buckets: usize) -> Self {
        Self {
            ptr: unsafe { hashmap::hashmap_with_buckets(buckets) },
            value: PhantomData,
        }
    }

    /// Returns the number of keys stored in the HashMap.
    pub fn len(&self) -> usize {
        unsafe { hashmap::hashmap_len(self.ptr) }
    }

    /// Returns true if the HashMap contains no elements.
    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }

    /// Returns the ratio of keys to buckets in the HashMap.
    pub fn load_factor(&self) -> f64 {
        unsafe { hashmap::hashmap_load_factor(self.ptr) }
    }

    /// Insert a key-value pair into the hashmap. If the key already exists, the
    /// old value is returned.
    pub fn insert(&mut self, key: &str, value: Box<V>) -> Option<Box<V>> {
        unsafe {
            let old_value = hashmap::hashmap_insert(
                self.ptr,
                CString::new(key).ok()?.as_ptr(),
                Box::into_raw(value) as *mut c_void,
            );
            if old_value.is_null() {
                None
            } else {
                Some(Box::from_raw(old_value as *mut _))
            }
        }
    }

    /// Get a reference to the value associated with the given key.
    pub fn get(&mut self, key: &str) -> Option<&V> {
        unsafe {
            let value = hashmap::hashmap_get(self.ptr, CString::new(key).ok()?.as_ptr());
            if value.is_null() {
                None
            } else {
                Some(&*(value as *const _))
            }
        }
    }

    /// Get a mutable reference to the value associated with the given key.
    ///
    /// # Safety
    /// Since the hashmap does not know if other references to the value exist,
    /// the safety of this operation is up to the caller.
    pub unsafe fn get_mut_unchecked(&mut self, key: &str) -> Option<&mut V> {
        unsafe {
            let value = hashmap::hashmap_get(self.ptr, CString::new(key).ok()?.as_ptr());
            if value.is_null() {
                None
            } else {
                Some(&mut *(value as *mut _))
            }
        }
    }

    /// Remove the value associated with the given key from the hashmap. The old
    /// value is returned.
    pub fn remove(&mut self, key: &str) -> Option<Box<V>> {
        unsafe {
            let value = hashmap::hashmap_remove(self.ptr, CString::new(key).ok()?.as_ptr());
            if value.is_null() {
                None
            } else {
                Some(Box::from_raw(value as *mut _))
            }
        }
    }
}

impl<V> Default for HashMap<V> {
    fn default() -> Self {
        Self {
            ptr: unsafe { hashmap::hashmap_new() },
            value: PhantomData,
        }
    }
}

impl<V> Drop for HashMap<V> {
    fn drop(&mut self) {
        unsafe { hashmap::hashmap_free(&mut self.ptr as *mut *mut _) }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_insert() {
        let mut map = HashMap::new();
        assert_eq!(map.insert("foo", Box::new("42")), None);
        assert_eq!(map.insert("foo", Box::new("43")), Some(Box::new("42")));
    }

    #[test]
    fn test_get() {
        let mut map = HashMap::new();
        assert_eq!(map.get("foo"), None);
        map.insert("foo", Box::new(42));
        assert_eq!(map.get("foo"), Some(&42));
    }

    #[test]
    fn test_remove() {
        let mut map = HashMap::new();
        assert_eq!(map.remove("foo"), None);
        map.insert("foo", Box::new(42));
        assert_eq!(map.remove("foo"), Some(Box::new(42)));
        assert_eq!(map.get("foo"), None);
        assert_eq!(map.remove("foo"), None);
    }

    #[test]
    fn test_get_mut_unchecked() {
        let mut map = HashMap::new();
        unsafe {
            assert_eq!(map.get_mut_unchecked("foo"), None);
        }
        map.insert("foo", Box::new(42));
        unsafe {
            assert_eq!(map.get_mut_unchecked("foo"), Some(&mut 42));
        }
    }

    #[test]
    fn test_many_insertions() {
        let mut map = HashMap::new();
        for i in 0..1000 {
            assert_eq!(map.insert(&i.to_string(), Box::new(i)), None);
        }
        for i in 0..1000 {
            assert_eq!(map.get(&i.to_string()), Some(&i));
        }
    }
}
